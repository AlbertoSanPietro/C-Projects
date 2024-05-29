/*
 *Il seguente programma è per una VM in architettura "LC-3": le parole sono di 16 bit per i registri e usiamo memoria indirizzabile a 16-bit con 2^16 spazi, "possiamo stoccare solo 128KB!"
 E' basato su: https://www.jmeiners.com/lc3-vm/
 *Il file di registro contiene 8 registri general purpose da "R0" a "R7", 1 è un program counter (PC), 1 è un condition flags (COND)
 *Le istruzioni sono di 16 bit e hanno "opcodes" da 4 bit
 *L'opcode è semplicemente la porzione di un istruzione in linguaggio macchina che specifica l operazione da eseguire
 *tastiera e monitori sono supportati tramite astrazioni memory mapped I/O
 *Gli I/O usano ASCII
*/

//IN QUESTA VERSIONE E' STATO VOLUTAMENTE RIMOSSO IL CODICE INTERNO AI VARI OPERATORI E SPOSTATO NELLO SWITCH CASE
//Includes necessari:
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/*solo su unix: */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define MEMORY_MAX (1<<16) //fancy bitwise il risultato è 2^16
uint16_t memory[MEMORY_MAX]; /*65536 spazi, è 2^16, la memoria è in questo array*/
//Registri, usiamo enum una struttura dati di integrali o integers COSTANTI definiti dall'utente. rendono il codice leggibile e mantenibile
enum 
{
	R_R0 = 0,
	R_R1,
	R_R2,
	R_R3,
	R_R4,
	R_R5,
	R_R6,
	R_R7,
	R_PC, // program counter
	R_COND, //condition flag
	R_COUNT
};

//stocchiamo i registri in un array
uint16_t reg[R_COUNT]; //uint16_t è uno uint short, ovvero un intero senza segno da 16 bit; ce ne sono altri tipi: uint8_t, uint16_t, uint32_t e uint64_t.

//OPCODES
enum
{
	OP_BR=0, /*branch*/
	OP_ADD, /*add*/
	OP_LD, /*load*/
	OP_ST, /*store*/
	OP_JSR, /*jump register, salta registro*/
	OP_AND, /*bitwise and*/
	OP_LDR, /*load register, carica registro*/
	OP_STR, /*store register*/
	OP_RTI, /*unused*/
	OP_NOT, /*bitwise not*/
	OP_LDI, /*load indirect*/
	OP_STI, /*store indirect*/
	OP_JMP, /*jump*/
	OP_RES, /*reserved (unused)*/
	OP_LEA, /*load effective address*/
	OP_TRAP /*execute trap, essenzialmente entra in campo se c'è un'eccezione e interrompe un'istruzione che non può essere restartata*/
};

//Condition flags: R_COND stocca delle flag sulle condizoni che danno informazioni sui calcoli eseguiti più recentemente. 
//Questo permette al programma di controllare condizoni logiche come "if (x>0) {codice}" 
//LC-3 ha solo 3 flags che indicano il segno del calcolo precedente
//Il "<<" è bitshift sinistro: (n<<k) sposta i bit di n sulla sinistra di k spazi. quindi 1 << 2 è uguale a 4
//
enum 
{
	FL_POS = 1 << 0, /*positivo*/
	FL_ZRO = 1 << 1, /*zero*/
	FL_NEG = 1 << 2, /*negativo*/
};

/*TRAP ROUTINES*/
enum
{
    TRAP_GETC = 0x20,  /* prendi un carattere da tastiera, non mostrato sul terminale (not echoed onto the terminal) */
    TRAP_OUT = 0x21,   /* outputta un carattere */
    TRAP_PUTS = 0x22,  /* butta fuori una stringa di parole (word string) */
    TRAP_IN = 0x23,    /* prendi un carattere da tastiera, mostrato sul terminale (echoed onto the terminal) */
    TRAP_PUTSP = 0x24, /* butta fuori una strina di byte */
    TRAP_HALT = 0x25   /* blocca il programma */
};
//KBSR e KBDR responsabili della gestione della tastiera
enum
{
    MR_KBSR = 0xFE00, /* status della tastiera */
    MR_KBDR = 0xFE02  /* dati della tastiera */
};

//Estensione del segno o Sign-extending
uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}
//da notare che swap16 è chiamate su ogni valore caricato. I programmi LC-3 sono in big endian, ma i computer moderni usano il little endian
uint16_t swap16(uint16_t x) {
	return (x << 8) | (x >> 8); //stiamo facendo un bitwise OR non un or normale "||" il risultato di OR è 1 se uno quasliasi dei due bit è 1
}

//OGNI VOLTA che un valore viene scritto sul registro dobbiamo fare un update delle flags per indicarne il segno
void update_flags(uint16_t r) 
{
	if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* un 1 nel bit più a sx indica il negativo */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

void read_image_file(FILE* file)
{
/* L'origine ci dice dove in memoria dobbiamo piazzare la immagine*/
    	uint16_t origin;
    	fread(&origin, sizeof(origin), 1, file);
    	origin = swap16(origin);

/*conosciamo le dimensioni massime del file quindi abbiamo bisogno solo di una fread*/
    	uint16_t max_read = MEMORY_MAX - origin;
    	uint16_t* p = memory + origin;
    	size_t read = fread(p, sizeof(uint16_t), max_read, file);

/* passiamo al little endian */
   	 while (read-- > 0)
    	{
        	*p = swap16(*p);
        	++p;
    	}
}

int read_image(const char* image_path){
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

//Istruzioni necessarie per LINUX/UNIX
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

void handle_interrupt(int signal)
{
    	restore_input_buffering();
    	printf("\n");
    	exit(-2);
}

/*void setup() {
	signal(SIGINT, handle_interrupt);
	disable_input_buffering();
}*/

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

//Scrivere con getter e setter sull'indirizzo speciali dei mmr per I/O
uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

/*Istruzioni:
//ADD:	
void op_add(uint16_t instr){}

//AND
void op_and(uint16_t instr) {
	
}
//NOT
void op_not(uint16_t instr) {
	
	
}
//BRANCH
void op_br(uint16_t instr) {
	
}
//JUMP
void op_jmp(uint16_t instr) {
	/*RET  è stata messa come istruzione a sè nelle specifiche, dato che è una keyword diversa in assembly,
	 * Tuttavia è solo un caso speciale di JMP che avviene quando R1 è 7*/
	/*gestisce anche RET
	
}
/*JUMP REGISTER
void op_jsr(uint16_t instr) {
	
}
//LOAD
void op_ld(uint16_t instr) {
	
}
//LOAD INDIRECT	
void op_ldi(uint16_t instr)
{


}
//LOAD IN REGISTER, CARICA  NEL REGISTRO	
void op_ldr(uint16_t instr) {
	
}
//LOAD EFFECTIVE ADDRESS
void op_lea(uint16_t instr) {
	
}
//STORE
void op_st(uint16_t instr) {
	
}
//STORE INDIRECTLY
void op_sti(uint16_t instr) {
	 
}
/*STORE REGISTER
void op_str(uint16_t instr) {
	
}*/

/*TRAP ROUTINES:
void trap_puts() {
/* un char per parola 
	uint16_t* c = memory + reg[R_R0];
    	while (*c)
    	{
        	putc((char)*c, stdout);
        	++c;
    	}
    	fflush(stdout); //flusha il buffer di output di uno stream, in questo caso di stdout
}
void trap_getc() {
//leggi un singolo carattere ASCII
	
} 
void trap_out() {
	
}
void trap_in() {
	printf("Enter a character:\t");
	
}
void trap_putsp() {
/*un char per byte (2 byte per parola)
 * qui dobbiamo tornare al formato big endian
	
}

void trap_halt() {
	

}*/



int main (int argc, const char* argv[]) 
{
//carichiamo gli argomenti: 
	if (argc < 2) {
	/*mostra stringa di utilizzo, se minore di due non abbiamo un'immagine da caricare*/
		printf("lc3 [image-file1] ...\n");
		exit(2);
	} 
	for(int k = 0; k < argc; ++k) {
		if (!read_image(argv[k])) {
		printf("Impossibile caricare il programma dal file: %s", argv[k]);
		exit(1);
		}
	}

	//setup, è anche in void setup
	signal(SIGINT, handle_interrupt);
	disable_input_buffering();
/*dato che solo un flag condizionale deve essere settato allo stesso tempo, settiamo zero*/
	reg[R_COND] = FL_ZRO;
/*Settiamo PC alla posizione iniziale, il default è 0x3000*/
	enum {PC_START = 0x3000 };
	reg[R_PC] = PC_START;
	
	//setup();
	int running = 1;
	while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_ADD:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] + imm5;
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] + reg[r2];
                    }
                    update_flags(r0);
                }
                break;
            case OP_AND:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                
                    if (imm_flag)
                    {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] & imm5;
                    }
                    else
                    {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] & reg[r2];
                    }
                    update_flags(r0);
                }
                break;
            case OP_NOT:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                
                    reg[r0] = ~reg[r1];
                    update_flags(r0);
                }
                break;
            case OP_BR:
                {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    if (cond_flag & reg[R_COND])
                    {
                        reg[R_PC] += pc_offset;
                    }
                }
                break;
            case OP_JMP:
                {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
                break;
            case OP_JSR:
                {
                    uint16_t long_flag = (instr >> 11) & 1;
                    reg[R_R7] = reg[R_PC];
                    if (long_flag)
                    {
                        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[R_PC] += long_pc_offset;
                    }
                    else
                    {
                        uint16_t r1 = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[r1];
                    }
                }
                break;
            case OP_LD:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = mem_read(reg[R_PC] + pc_offset);
                    update_flags(r0);
                }
                break;
            case OP_LDI:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                    update_flags(r0);
                }
                break;
            case OP_LDR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    reg[r0] = mem_read(reg[r1] + offset);
                    update_flags(r0);
                }
                break;
            case OP_LEA:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = reg[R_PC] + pc_offset;
                    update_flags(r0);
                }
                break;
            case OP_ST:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(reg[R_PC] + pc_offset, reg[r0]);
                }
                break;
            case OP_STI:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
                }
                break;
            case OP_STR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(reg[r1] + offset, reg[r0]);
                }
                break;
            case OP_TRAP:
                reg[R_R7] = reg[R_PC];
                
                switch (instr & 0xFF)
                {
                    case TRAP_GETC:
                        reg[R_R0] = (uint16_t)getchar();
                        update_flags(R_R0);
                        break;
                    case TRAP_OUT:
                        putc((char)reg[R_R0], stdout);
                        fflush(stdout);
                        break;
                    case TRAP_PUTS:
                        {
                            /* 1 char per parola */
                            uint16_t* c = memory + reg[R_R0];
                            while (*c)
                            {
                                putc((char)*c, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }
                        break;
                    case TRAP_IN:
                        {
                            printf("Enter a character: ");
                            char c = getchar();
                            putc(c, stdout);
                            fflush(stdout);
                            reg[R_R0] = (uint16_t)c;
                            update_flags(R_R0);
                        }
                        break;
                    case TRAP_PUTSP:
                        {
                            uint16_t* c = memory + reg[R_R0];
                            while (*c)
                            {
                                char char1 = (*c) & 0xFF;
                                putc(char1, stdout);
                                char char2 = (*c) >> 8;
                                if (char2) putc(char2, stdout);
                                ++c;
                            }
                            fflush(stdout);
                        }
                        break;
                    case TRAP_HALT:
                        puts("Sistema arrestato");
                        fflush(stdout);
                        running = 0;
                        break;
                }
                break;
            case OP_RES:
            case OP_RTI:
            default:
                abort();
                break;
        }
    }
restore_input_buffering();
}






