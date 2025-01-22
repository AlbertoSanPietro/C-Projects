//Albero binario per il conteggio delle parole ripetute, ordinate per lunghezza. A sx se più corte della parola prec, a dx se più lunghe, sulla base di K&R
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAXW 50 //massima lunghezza della parola
#define BUFS 1000

struct tnode { //il nodo dell'albero
	char *word; //ptr al testo
	int count; //contatore
	struct tnode *left;//figlio sx
	struct tnode *right; //figlio dx
};
typedef struct tnode tnode;

tnode *mytalloc(void);
char *mystrdup(char *);
tnode *addtree(tnode *, char *); //la fz è addtree ed è di tipo tnode
void treeprint(tnode *); //l'argomento è un ptr di tipo tnode!

/*contatore della frequenza delle parole*/
int main() {
	tnode *root;
	char word[MAXW]; 
	char *ptr;
	root = NULL;	
	while(fgets(word, sizeof(word), stdin)!=NULL){
		int i=0; //ptr=word;
		while (isalpha(word[i++])!=0) //while(isalpha(*ptr++)) !=0)
			;//isalpha ritorna 0 se il parametro non è alfabetico e non 0 se è alfabetico 
		if (word[i] == '\0'){ //*ptr == '\0'
			root = addtree(root, word); //aggiunge all'albero
		}
	}
	treeprint(root);
	return 0;
}


/*addtree aggiungi un nodo con w, al livello o sotto p*/
tnode *addtree( tnode *p, char *w)
{
	int cond;
	if (p == NULL) {//c'è una nuova parola
		//p = malloc(sizeof(tnode)); //crea un nuovo nodo
		p=mytalloc();
		p->word = mystrdup(w); //la sintassi per gli elementi  è NomeStruttura.Membro, se abbiamo un ptr possiamo o fare *(*NomeStruttura).Membro oppure ptr->MembroStruttura  
		p->count = 1; //se nestato diventa NomeStruttura.NomeSottoStruttura.Membro e così via
		p->left = p->right = NULL;
		return p;
	} else if ((cond = strcmp(w, p->word)) == 0){
		p->count++; //parola ripetuta
	}  else if (cond < 0) {//minore quindi albero sx
		
		p->left = addtree(p->left, w);
	} else {
		p->right = addtree(p->right, w); //albero dx
	}	
	return p;

}

//treeprint: stampa i rami dell'albero
void treeprint( tnode *p) {
	if (p != NULL) {
		treeprint(p->left);
		printf("%4d %s\n", p->count, p->word);
		treeprint(p->right);
	}
}


tnode *mytalloc(void) {
	return (tnode *) malloc(sizeof(tnode));
}

char *mystrdup(char *s) { //duplica s, strdup= string duplicate
	char *p;
	p = (char *) malloc(strlen(s)+1); //per gestire il null terminator
	if (p!=NULL)
		strcpy(p,s);
	return p;	
} 







