// Parcial 1 SO
// Juan Camilo Vasquez Gonzalez
// 2223327-3744
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_P 100

typedef struct {
    char etiqueta[10];
    int BT;
    int AT;
    int Q;
    int Pr;
    int WT;
    int CT;
    int RT;
    int TAT;
    int I; // Indicador de ejecucion del proceso
    int BT_DD; // Para no alterar BT
} Proceso;

void lecturadocumento(const char *nombredocumento, Proceso procesos[], int *numprocesos) {
    FILE *documento = fopen(nombredocumento, "r");
    if (!documento) {
        perror("Error documento no encontrado.");
        exit(EXIT_FAILURE);
    } else {
        printf("OK\n");
    }
    char linea[256];
    int numlinea = 0;
    // Lectura de datos
    while (fgets(linea, sizeof(linea), documento)) {
        if (linea[0] == '#' || linea[0] == '\n') {
            continue; // Lineas de comentario o vacías
        }
        char tempEtiqueta[10];
        sscanf(linea, "%9[^;]; %d; %d; %d; %d",
               tempEtiqueta,
               &procesos[numlinea].BT,
               &procesos[numlinea].AT,
               &procesos[numlinea].Q,
               &procesos[numlinea].Pr);
        strncpy(procesos[numlinea].etiqueta, tempEtiqueta, sizeof(procesos[numlinea].etiqueta));
        procesos[numlinea].WT = 0;
        procesos[numlinea].CT = 0;
        procesos[numlinea].RT = 0;
        procesos[numlinea].TAT = 0;
        procesos[numlinea].I = 1;
        procesos[numlinea].BT_DD = procesos[numlinea].BT;
        numlinea++;
    }
    *numprocesos = numlinea;
    fclose(documento);
}

// Se ordena por prioridad las colas (5 > 1)
void ordenar_colas(Proceso *cola[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (cola[j]->Pr < cola[j + 1]->Pr) {
                Proceso *temp = cola[j];
                cola[j] = cola[j + 1];
                cola[j + 1] = temp;
            }
        }
    }
}

// Se planifica los procesos
void planificar(Proceso procesos[], int numprocesos) {
    int tiempo = 0;
    int procesos_por_ejecutar1 = numprocesos;
    char x[MAX_P];//Proceso de control para el caso 2
    // Se crean las tres colas
    Proceso *cola1[MAX_P], *cola2[MAX_P], *cola3[MAX_P];
    int n1 = 0, n2 = 0, n3 = 0;
    // Clasificacion de procesos por cola
    for (int i = 0; i < numprocesos; i++) {
        if (procesos[i].Q == 1) {
            cola1[n1++] = &procesos[i];
        } else if (procesos[i].Q == 2) {
            cola2[n2++] = &procesos[i];
        } else {
            cola3[n3++] = &procesos[i];
        }
    }
    // Ordenar colas por prioridad
    ordenar_colas(cola1, n1);
    ordenar_colas(cola2, n2);
    ordenar_colas(cola3, n3);
    while (procesos_por_ejecutar1 > 0) {
        int ejecutado = 0;
        int ejecutado2 = 0;
        // Cola 1 RR(1)
        for (int i = 0; i < n1; i++) {
            Proceso *p = cola1[i];
            if (p->BT_DD > 0 && p->AT <= tiempo) {
                // Se identifica el tiempo RT, primera vez ejecutado el proceso
                if (p->I == 1) {
                    p->RT = tiempo;
                    p->I = 0;
                }
                // Se ejecuta la linea de tiempo y se define el nuevo tiempo
                int ejecutado_tiempo = (p->BT_DD > 1) ? 1 : p->BT_DD;
                p->BT_DD -= ejecutado_tiempo;
                tiempo += ejecutado_tiempo;
                // Se identifica el tiempo de completado del proceso en ejecucion y se calculan las metricas
                if (p->BT_DD == 0) {
                    p->CT = tiempo;
                    p->TAT = p->CT - p->AT;
                    p->WT = p->CT - p->AT - p->BT;
                    procesos_por_ejecutar1--;
                }
                
                ejecutado = 1;
            }
        }
        // Cola 2 RR(3)
        if (!ejecutado) {
            for (int i =  0; i < n2; i++) {
                
                Proceso *p = cola2[i];
                if (p->BT_DD > 0 && p->AT <= tiempo && p->I == 1) {
                    if (p->RT == 0) {
                        p->RT = tiempo;
                    }
                    int ejecutado_tiempo = (p->BT_DD > 3) ? 3 : p->BT_DD;
                    p->BT_DD -= ejecutado_tiempo;
                    tiempo += ejecutado_tiempo;
                    if (p->BT_DD == 0) {
                        p->CT = tiempo;
                        p->TAT = p->CT - p->AT;
                        p->WT = p->CT - p->AT - p->BT;
                        procesos_por_ejecutar1--;
                    }
                    x[i] = n2 - i;
                    
                    if (x[i] == 1){
                        p->I = 1;
                    }else{
                        p->I = 0;
                    }
                    
                    ejecutado = 1;
                    break;
                } else if (p->BT_DD > 0 && p->AT <= tiempo && p->I == 0) {//Proceso de control, para identificar el proceso a ejecutar
                    x[i] -= 1;
                    if (x[i] == 1){
                        p->I = 1;
                    }
                    ejecutado2 = 1;
                    
                }

                                
            }
        }

        // Cola 3 SJF
        if (!ejecutado && !ejecutado2) {
            // Buscar el proceso con el menor BT_DD que haya llegado
            Proceso *proceso_sjf = NULL;
            for (int i = 0; i < n3; i++) {
                Proceso *p = cola3[i];
                if (p->BT_DD > 0 && p->AT <= tiempo) {
                    if (proceso_sjf == NULL || p->BT_DD < proceso_sjf->BT_DD) {
                        proceso_sjf = p;
                    }
                }
            }
            if (proceso_sjf != NULL) {
                Proceso *p = proceso_sjf;
                if (p->I == 1) {
                    p->RT = tiempo;
                    p->I = 0;
                }
                // Ejecución de proceso y cálculo de parámetros
                p->WT = tiempo - p->AT;
                tiempo += p->BT_DD;
                p->BT_DD = 0;
                p->CT = tiempo;
                p->TAT = p->CT - p->AT;
                procesos_por_ejecutar1--;
                ejecutado = 1;
            }
        }
        
        // Si no se ejecuta ningún proceso, el tiempo avanza
        if (!ejecutado && !ejecutado2) {
            tiempo++;
        }
    }
}

// Calculo de promedio
void metricas_promedio(Proceso procesos[], int numprocesos, float *WTP, float *CTP, float *RTP, float *TATP) {

    int WTT = 0, CTT = 0, RTT = 0, TATT = 0; // Totales

    for (int i = 0; i < numprocesos; i++) {// Sumas totales de todas las metricas calculadas
        WTT += procesos[i].WT;
        CTT += procesos[i].CT;
        RTT += procesos[i].RT;
        TATT += procesos[i].TAT;
    }

    *WTP = (float)WTT / numprocesos;
    *CTP = (float)CTT / numprocesos;
    *RTP = (float)RTT / numprocesos;
    *TATP = (float)TATT / numprocesos;
}

// Generacion de archivo de salida
void resultados(const char *nombre_archivo, char nombredocumento[], Proceso procesos[], int numprocesos, float WTP, float CTP, float RTP, float TATP) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        perror("Error al abrir el archivo de salida");
        exit(EXIT_FAILURE);
    }

    fprintf(archivo, "# archivo: %s\n", nombredocumento);
    fprintf(archivo, "# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT\n");

    for (int i = 0; i < numprocesos; i++) {
        fprintf(archivo, "%s;%d;%d;%d;%d;%d;%d;%d;%d\n",
                procesos[i].etiqueta,
                procesos[i].BT,
                procesos[i].AT,
                procesos[i].Q,
                procesos[i].Pr,
                procesos[i].WT,
                procesos[i].CT,
                procesos[i].RT,
                procesos[i].TAT);
    }

    fprintf(archivo, "WT=%.1f; CT=%.1f; RT=%.1f; TAT=%.1f; Promedio de las métricas.\n", WTP, CTP, RTP, TATP);

    fclose(archivo);
}



int main() {
    Proceso procesos[MAX_P];
    int numprocesos;
    char nombredocumento[100];
    float WTP, CTP, RTP, TATP;
    printf("Ingrese el nombre del archivo o la ruta:\n");
    scanf("%s", nombredocumento);
    lecturadocumento(nombredocumento, procesos, &numprocesos);
    printf("Numero de procesos: %d\n", numprocesos);
    
    planificar(procesos, numprocesos);
    
    metricas_promedio(procesos, numprocesos, &WTP, &CTP, &RTP, &TATP);
    
    resultados("Resultados.txt", nombredocumento, procesos, numprocesos, WTP, CTP, RTP, TATP);

    printf("El archivo 'Resultados.txt' ha sido generado exitosamente.\n");

    return 0;
}
