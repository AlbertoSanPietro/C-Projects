//Inverse polish notation calculator
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define MAXOP 100
#define MAXVAL 100
#define BUFFSIZE 100
#define NUMBER '0'

int getop(char[]);

void push(double f);
double pop(void);
void view_head(void);
void duplicate(void);
void swap(void);
void clear(void);
char s[MAXOP];
int main(void) {
  
  printf("Questo è un calcolatore in notazione Polacca inversa.\n");
  printf("Ecco un esempio: (1-2) * (4+5) diventa 1 2 - 4 5 + *\n");
  printf("Usare gli spazi tra cifre e operatori, il limite è 100 char\n");
  printf("Supporta le 4 operazioni più resto (%), elevamento a potenza (^), seno (~), esponente (e)\n.");
  printf("Ecco un elenco di comandi:\n h: vedi la testa della stack,\n c: pulisci la stack,\n d: duplica,\n s: swap o scambio\n");
  printf("Per uscire, se sei su terminale usa Ctrl+D sotto Linux e Ctrl+Z sotto windows\n");
  printf("Inserisci qui l'input:\n");
	
  int type;
  double op2;
  //char s[MAXOP];

  while ((type = getop(s)) != EOF)
  {
    switch (type)
    {
    case NUMBER:
      push(atof(s));
      break;

    case '+':
      push(pop() + pop());
      break;

    case '-':
      op2 = pop();
      push(pop() - op2);
      break;

    case '*':
      push(pop() * pop());
      break;

    case '/':
      op2 = pop();

      if (op2 != 0.0)
      {
        push(pop() / op2);
      }
      else
      {
        printf("Error: zero divisor.\n");
      }

      break;

    case '%':
      op2 = pop();

      if (op2 != 0.0)
      {
        push((int)pop() % (int)op2);
      }
      else
      {
        printf("Error: zero divisor.\n");
      }
      break;

    case '^':
      op2 = pop();
      push(pow(pop(), op2));
      break;

    case '~':
      push(sin(pop()));
      break;

    case 'e':
      push(exp(pop()));
      break;

    case 'h':
      view_head();
      break;

    case 'd':
      duplicate();
      break;

    case 's':
      swap();
      break;

    case 'c':
      clear();
      break;

    case '\n':
      printf("risultato: %.8g\n", pop());
      break;

    default:
      printf("Errore: comando ignoto! %s\n", s);
      break;
    }
  }

  return 0;
}


int sp = 0; //prossima posizione libera sulla stack
//stack del valore
double stack[MAXVAL]; 
//spinge f sulla stack dei valori
void push (double f) { 
	if (sp<MAXVAL)
		stack[sp++] = f;
	else
		printf("Errore, stack piena.\n");
}

//controllo che la stack abbia dei valori e ritorno l'input 
double pop (void) { 
        //for (int k =0; k<=sp; ++k) printf("%c", s[k]); //stampa i char sulla stack 
        if (sp>0)
		return stack[--sp];
	else {
		printf("errore: stack vuota\n");
		return 0.0;
	}
}

void view_head(void) {
	if (sp)
		printf("stack_head: %g\n", stack[sp-1]);
	else
		printf("Errore: no head, stack vuota");

}
//duplico gli elementi in cima alla stack
void duplicate(void) {
	double temp= pop();
	push(temp);
	push(temp);
}

void swap(void) { // scambia gli elementi
	double temp1=pop();
	double temp2=pop();
	push(temp1);
	push(temp2);
}


//Puliamo la stack
void clear (void) {
        sp=0;
}

int bufp=-1;
char buf[BUFFSIZE];

int getch(void) {
	return (bufp>0)? buf[--bufp] : getchar();
}

void ungetch(int c) {
	if (bufp>=BUFFSIZE)
		printf("Troppi caratteri, cancello gli ultimi\n");
	else
		buf[bufp++]=c;
}



int getop(char s[]) //Prende gli operatoi. get operator
{
	int i = 0, c;
	while ((s[0] = c = getch()) == ' ' || c=='\t')
		//printf("sp= %d", sp);
		;
	s[1]= '\0';

	
	
	if (!isdigit(c) && c != '.' && c !='-')
		return c; //Se C non è un '.' (inizio decimale) o un '-' (inizio num neg) ritorna c

	if (c=='-') {
		//int next = getch();
	//if (next == '\n' || next == ' ' || next == '\t') {
		if(!isdigit(s[++i]=c=getch())) {
		ungetch(c);
		c=s[0];
		//return c; //ritorna '-' come operatore se il char dopo è uno spazio, invio o tab
	}
	if (!isdigit(c) && c != '.')
		return c; //non un numero
	else if (c== '.') /*numeri come -5, -10, -.34 ecc. dopo c'è una cifra o un '.'*/ {
	     	s[i++] = c = getch();
	      } 
	
	}
	if (isdigit(c)) {
		while (isdigit(s[++i] = c = getch()))
			;
	}
	if (c == '.'){
			while(isdigit(s[++i] = c = getch()))
				;
		}
	//s[++i] = '\0';

	if (c!=EOF) {
		ungetch(c);
	}
	for (int k =0; k<=sp; ++k) printf("%c", s[k]); 	
	printf("\n");
	for (int p =0; p<=BUFFSIZE; ++p) printf("%c", buf[p]); 
	return NUMBER;
}
