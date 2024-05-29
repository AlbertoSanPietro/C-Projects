#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#define MAXL 10000
void qsort(int v[], int sx, int dx);
void swap(int v[], int i, int j);
void print(int v[], int n);
void dualPivotQS(int v[], int sx, int dx);
void doSort(int v[], int sx, int dx);

int main() 
{
	int vett[MAXL];
	double time1, timedif;
/*
Ordinamento di dati interni ad un file	
FILE *myFile;
    myFile = fopen("somenumbers.txt", "r");

    //read file into array
    int i;

    for (i = 0; i < MAXL; i++)
    {
        fscanf(myFile, "%d", &vett[i]);
    }

    for (i = 0; i < 16; i++)
    {
        printf("Number is: %d\n\n", vett[i]);
    }*/	
	
	
	int n=MAXL;
//	dimensione array
	printf("Inserire la dimesione dell'array\n");
	scanf("%d", &n);
	//elementi array	
	printf("\nInserire gli elementi dell'array:\n");
	for(int i=0; i<n; i++){
		scanf("%d", &vett[i]);
	}
	
	dualPivotQS(vett, 0, n-1);
	timedif = ( ((double) clock()) / CLOCKS_PER_SEC) - time1;
    	printf("The elapsed time is %lf seconds\n", timedif);	
	printf("\n Print finale:\n");
	print(vett, n);
	
}

//Implementazione del QuickSort
void qsort(int vettore[], int sinistra, int destra)
{
	int i = 0, ultimo;
	
	if (sinistra >= destra)
		return; //se entrambi le parti contengono 2 o meno elem fermati non fare nulla
	swap(vettore, sinistra, (sinistra+destra)/2); //muovi l'elem di partizione fino a v[0]
	ultimo=sinistra;
	for (i = sinistra+1; i<=destra; i++){ //partizione
		if(vettore[i] < vettore[sinistra])
			swap (vettore, ++ultimo, i);
	}
	swap (vettore, sinistra, ultimo);	//rimettiamo a posto l'elem di partizione
	qsort(vettore, sinistra, ultimo-1);
	qsort (vettore, ultimo+1, destra);
}

//Swap, effettua uno scambio
void swap (int v[], int i, int j)
{
	int temp;
	temp=v[i];
	v[i]=v[j];
	v[j]=temp;
}

//print, stampa
void print(int v[], int n) 
{
	printf("\n");
	for (int i=0; i<n;i++)
		printf("%d\t", v[i]);
}


void dualPivotQS(int v[], int sx, int dx){
	unsigned sesto = (dx-sx+1)/6;
	unsigned int e1 = sx + sesto;
	unsigned int e5 = dx - sesto;
	unsigned int e3 = (dx+sx) >> 1; //Il punto centrale
	unsigned int e4 = e3 + sesto;
	unsigned int e2= e3 - sesto;
	//Ordiniamo questi elementi
	int ve1 = v[e1], ve2=v[e2], ve3=v[e3], ve4=v[e4], ve5=v[e5];
	
		if (ve1 > ve2) { int t = ve1; ve1 = ve2; ve2 = t; }
        if (ve4 > ve5) { int t = ve4; ve4 = ve5; ve5 = t; }
        if (ve1 > ve3) { int t = ve1; ve1 = ve3; ve3 = t; }
        if (ve2 > ve3) { int t = ve2; ve2 = ve3; ve3 = t; }
        if (ve1 > ve4) { int t = ve1; ve1 = ve4; ve4 = t; }
        if (ve3 > ve4) { int t = ve3; ve3 = ve4; ve4 = t; }
        if (ve2 > ve5) { int t = ve2; ve2 = ve5; ve5 = t; }
        if (ve2 > ve3) { int t = ve2; ve2 = ve3; ve3 = t; }
        if (ve4 > ve5) { int t = ve4; ve4 = ve5; ve5 = t; }
	
	v[e1] = ve1; v[e3] = ve3; v[e5] = ve5;

	int pivot1 = ve2; v[e2] = v[sx];
	int pivot2 = ve4; v[e4] = v[dx]; 
	int less = sx+1;
	int great = dx-1;
	
	//I pivot sono diversi
	bool pivotsDiffer = (pivot1 != pivot2);
	if (pivotsDiffer) {
here1:
		for (int k = less; k<=great; k++){
			int vk=v[k];
			if (vk < pivot1) {
				if (k != less){
					v[k]=v[less];
					v[less] = vk;
				}
				less++;
				
			} else if (vk > pivot2) {
				while (v[great]>pivot2){
					if (great-- == k){
						goto here1;
					}
				}
			
			if (v[great] < pivot1) {
				v[k]=v[less];
				v[less++]=v[great];
				v[great--]= vk;
			} else {
				v[k]=v[great];
				v[great--] = vk;
				}	
				
			} 
	
		}
//here1:	
	}
	else /*I pivot sono uguali*/ {
		for (int k=less; k<=great; k++) {
			int vk = v[k];
			if (vk == pivot1){
				continue;
			}
			if (vk < pivot1) {
				if (k != less) {
					v[k] = v[less];
					v[less] = vk;
				}
			less++;
			} else {
				while (v[great] > pivot1) {
					great--;
				}
			
				if (v[great] < pivot1) {
					v[k]=v[less];
					v[less++]=v[great];
					v[great--]=vk;
				} else {
					v[k] = pivot1;
					v[great--] = vk;
				}
			}

		}
	}	//scambia i pivot
		v[sx] = v[less -1]; v[less -1] = pivot1;
		v[dx] = v[great +1]; v[great +1] = pivot2; 

		//sorting
		doSort(v, sx, less-2);
		doSort(v, great+2, dx);

		if (!pivotsDiffer) {
			return;
		}
		//Parte centrale troppo grande
		if (less < e1 && great > e5) {
			while(v[less] == pivot1) {
				less++;
			}
			while(v[great] == pivot2) {
				great--;
			}
			here2: 
			for (int k = less; k <= great; k++) {
                int vk = v[k];
                if (vk == pivot2) { // Move a[k] to right part
                    while (v[great] == pivot2) {
                        if (great-- == k) {
                            goto here2;
                        }
                    }
                    if (v[great] == pivot1) {
                        v[k] = v[less];
                        v[less++] = pivot1;
                    } else { // pivot1 < a[great] < pivot2
                        v[k] = v[great];
                    }
                    v[great--] = pivot2;
                } else if (vk == pivot1) { // Move a[k] to left part
                    v[k] = v[less];
                    v[less++] = pivot1;
                }
            }
		}
		//Sorting(v, less, great);	
		doSort(v, less, great);
	 
}

//Chiama l'algoritmo di sorting più adatto
void doSort(int v[], int sx, int dx)
{
	int n =(dx-sx+1);
	if (n < 32) {
		qsort(v, sx, dx);
	
	} else {
		dualPivotQS(v, sx, dx);
	}

}



