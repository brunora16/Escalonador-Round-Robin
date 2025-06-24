#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define quantum 3
#define MAX_PROCESSOS 5
#define LIMITE_ESPERA 10
#define ALTA 1
#define BAIXA 0

// Estrutura de processos
typedef struct processo{
    int tempoChegada;
    int prioridade;
    int PID;
    int status;
    int tempoServico;
    int feedback;
    char necessidadeIO;
    int tempoIO;
    int tempoInicioIO; 
    int tempoEspera;
    int turnaround;
    int tempoChegadaOriginal;
    struct processo* prox;
} Processo;

/*
    Status Processo:
    0 = Novo
    1 = Retorna I/O
    2 = Preempção
    3 = Finalizado
*/

/* 
    Necessidade IO:
    A = Disco
    B = Fita Magnética
    C = Impressora
    D = Sem IO
*/

// Criar fila de processos
typedef struct {
    Processo* ini;
    Processo* fim;
    int tamanho;
} Fila;

// Criando as duas filas de prioridade
typedef struct {
    Fila* alta;
    Fila* baixa;
} FilaProcessos;

void initFila(Fila *f){
    f->ini = NULL;
    f->fim = NULL;
    f->tamanho = 0;
}

void initFilaProcessos(FilaProcessos *fp){
    fp->alta = (Fila*) malloc(sizeof(Fila));
    fp->baixa = (Fila*) malloc(sizeof(Fila));
    initFila(fp->alta);
    initFila(fp->baixa);
}

bool fila_vazia(Fila *f){
    return (f->tamanho == 0);
}

bool fp_vazia(FilaProcessos *fp){
    return (fila_vazia(fp->alta) && fila_vazia(fp->baixa));
}

void enqueue(Fila *f, Processo *p){
    p->prox = NULL;

    if(fila_vazia(f))
        f->ini = f->fim = p;
    else {
        f->fim->prox = p;
        f->fim = p;
    }

    f->tamanho++;
}

Processo* dequeue(Fila *f){
    if(fila_vazia(f)) 
        return NULL; // Fila já estava vazia

    Processo *temp = f->ini;
    f->ini = f->ini->prox;

    if(f->ini == NULL)
        f->fim = NULL;
  
    f->tamanho--;
    temp->prox = NULL;
    return temp; // Sucesso    
}

void enqueueFilaProcessos(FilaProcessos *fp, Processo *p){
    if(p->prioridade == BAIXA)
        enqueue(fp->baixa, p); // Baixa prioridade

    else{
        enqueue(fp->alta, p); // Alta prioridade
    }
}

Processo* dequeueFilaProcessos(FilaProcessos *fp){
    if(!fila_vazia(fp->alta)){
        return dequeue(fp->alta);
    }
    return dequeue(fp->baixa);
}

void liberarFila(Fila *f){
    Processo *atual = f->ini;
    
    while(atual != NULL){
        Processo *temp = atual;
        atual = atual->prox;
        free(temp);
    }

    free(f);
}

void liberarFilaProcessos(FilaProcessos *fp){
    liberarFila(fp->alta);
    liberarFila(fp->baixa);
    free(fp);
}

void feedback(FilaProcessos *fp) {
    Processo *p = fp->baixa->ini;
    Processo *anterior = NULL;

    while(p != NULL){
        p->tempoEspera++;

        if(p->tempoEspera >= LIMITE_ESPERA && p->prioridade == BAIXA){
            Processo *promovido = p;
            
            // Remove da fila baixa
            if(anterior == NULL)
                fp->baixa->ini = p->prox;
            else
                anterior->prox = p->prox;

            if(fp->baixa->fim == promovido)
                fp->baixa->fim = anterior;
            fp->baixa->tamanho--;

            // Aumenta a prioridade
            promovido->prioridade = ALTA;
            promovido->tempoEspera = 0;
            enqueueFilaProcessos(fp, promovido);
            printf("Feedback: Processo %d promovido para ALTA prioridade por espera excessiva (aging)\n", promovido->PID);

            if(anterior == NULL)
                p = fp->baixa->ini;
            else
                p = anterior->prox;
            
        } else {
            anterior = p;
            p = p->prox;
        }
    }
    
    for(Processo *p = fp->alta->ini; p!= NULL; p = p->prox)
        p->tempoEspera = 0;
}

void mostrarFilas(FilaProcessos *fp, Fila *filaIO, int tempo) {
    printf("\n[Tempo %d] FILA DE PRONTOS:\n", tempo);
    printf("  Alta prioridade: ");
    Processo *p = fp->alta->ini;

    while (p != NULL) {
        printf("[PID %d | Restante %d] -> ", p->PID, p->tempoServico);
        p = p->prox;
    }

    printf("NULL\n");

    printf("  Baixa prioridade: ");
    p = fp->baixa->ini;

    while (p != NULL) {
        printf("[PID %d | Restante %d] -> ", p->PID, p->tempoServico);
        p = p->prox;
    }
    printf("NULL\n");

    printf("  FILA DE I/O: ");

    p = filaIO->ini;

    while (p != NULL) {
        printf("  PID %d | Tipo I/O: %c | Retorno previsto: t=%d\n", p->PID, p->necessidadeIO, p->tempoChegada);
        p = p->prox;
    }

    if (fila_vazia(filaIO)) printf("(vazia)\n");

    printf("------------------------------------------------------------\n");
}


void escalonador(FilaProcessos *fp, Processo *listaProcessos, int qntdProcessos, int *tempoOciosoCPU) {
    int tempo = 0;
    int proximo = 0;
    int processosRestantes = qntdProcessos;

    Fila *filaIO = (Fila*) malloc(sizeof(Fila));
    initFila(filaIO);

    while (processosRestantes > 0) {

        // 1. Adiciona novos processos (ordem: NOVOS primeiro)
        while (proximo < qntdProcessos && listaProcessos[proximo].tempoChegada <= tempo) {
            enqueueFilaProcessos(fp, &listaProcessos[proximo]);
            printf("Tempo %d: Processo %d entrou na fila de ALTA prioridade (novo)\n", tempo, listaProcessos[proximo].PID);
            proximo++;
        }

        // 2. Retorno de I/O
        while (!fila_vazia(filaIO) && filaIO->ini->tempoChegada <= tempo) {
            Processo *procIO = dequeue(filaIO);

            if (procIO->necessidadeIO == 'A') {
                procIO->prioridade = BAIXA;
                printf("Tempo %d: Processo %d retornou de I/O (DISCO), vai para fila BAIXA\n", tempo, procIO->PID);
            } else {
                procIO->prioridade = ALTA;
                printf("Tempo %d: Processo %d retornou de I/O (%c), vai para fila ALTA\n", tempo, procIO->PID, procIO->necessidadeIO);
            }

            procIO->status = 2;
            enqueueFilaProcessos(fp, procIO);
        }

        // 3. Mostra o estado atual das filas
        mostrarFilas(fp, filaIO, tempo);

        // 4. Aplica feedback de prioridade
        feedback(fp);

        // 5. Se não há processos prontos, avança o tempo
        if (fp_vazia(fp)) {
            tempo++;
            (*tempoOciosoCPU)++;
            continue;
        }

        // 6. Escolhe próximo processo para executar
        Processo *p = dequeueFilaProcessos(fp);
        int tempoExec = quantum;

        // calcula se o I/O precisa começar neste quantum
        if (p->necessidadeIO != 'D' && tempo < p->tempoInicioIO && p->tempoInicioIO - tempo < tempoExec){
            tempoExec = p->tempoInicioIO - tempo;
        }

        // Se o processo tiver menos tempo de serviço restante, ajusta
        if (p->tempoServico < tempoExec)
            tempoExec = p->tempoServico;

        printf("Tempo %d: EXECUTANDO Processo %d (prioridade %s) por %d unidades\n",
               tempo, p->PID, p->prioridade == ALTA ? "ALTA" : "BAIXA", tempoExec);

        p->tempoServico -= tempoExec;

        // Decide o destino após execução
        if (p->necessidadeIO != 'D' && p->tempoInicioIO != -1 && tempo + tempoExec >= p->tempoInicioIO && p->tempoServico > 0) {
            // Simula envio para I/O
            p->status = 1;
            p->tempoChegada = tempo + tempoExec + p->tempoIO;
            enqueue(filaIO, p);
            printf("Processo %d foi para I/O (%c), retorna em t=%d\n", p->PID, p->necessidadeIO, p->tempoChegada);
            p->tempoInicioIO = -1;
        }
        else if (p->tempoServico > 0) {
            p->status = 2; // Preempção
            if (p->prioridade == ALTA)
                p->prioridade = BAIXA;
            enqueueFilaProcessos(fp, p);
            printf("Processo %d foi preemptado, volta para fila %s\n",
                   p->PID, p->prioridade == ALTA ? "ALTA" : "BAIXA");
        }
        else {
            p->status = 3; // Finalizado
            processosRestantes--;
            printf("Processo %d FINALIZADO\n", p->PID);
            p->turnaround = tempo - p->tempoChegadaOriginal;
        }

        tempo += tempoExec;
    }

    free(filaIO);
}

Processo* lerProcessosArquivos(char* arquivo, int qntd){
    FILE *f = fopen(arquivo, "r");
    if(!f){
        printf("Erro ao abrir arquivo\n");
        exit(1);
    }

    Processo *arrayProcessos = (Processo*) malloc (qntd * sizeof(Processo));
    if(!arrayProcessos){
        printf("Erro na alocacao de memoria\n");
        exit(1);
    }

    for (int i = 0; i < qntd; i++) {
        fscanf(f, "%d %d %d %d %d %d %c %d %d %d",
            &arrayProcessos[i].PID,
            &arrayProcessos[i].tempoChegada,
            &arrayProcessos[i].prioridade,
            &arrayProcessos[i].status,
            &arrayProcessos[i].tempoServico,
            &arrayProcessos[i].feedback,
            &arrayProcessos[i].necessidadeIO,
            &arrayProcessos[i].tempoIO,
            &arrayProcessos[i].tempoInicioIO,
            &arrayProcessos[i].tempoEspera
        );
        arrayProcessos[i].turnaround = 0;
        arrayProcessos[i].tempoChegadaOriginal = arrayProcessos[i].tempoChegada;
        arrayProcessos[i].prox = NULL;
    }
    
    fclose(f);
    return arrayProcessos;
}

// Função para trocar os processos
void swap(Processo *a, Processo *b){
    Processo temp = *b;
    *b = *a;
    *a = temp;
}

void bubbleSortPID(Processo *array, int tamanho) {
    for (int i = 0; i < tamanho - 1; i++) {
        for (int j = 0; j < tamanho - 1 - i; j++) {
            if(array[j].PID > array[j+1].PID) 
                swap(&array[j], &array[j+1]);  
        }
    }
}

void bubbleSortNovos(Processo *array, int tamanho) {
    for (int i = 0; i < tamanho - 1; i++) {
        for (int j = 0; j < tamanho - 1 - i; j++) {
            if(
                array[j].tempoChegada > array[j + 1].tempoChegada ||
                (array[j].tempoChegada == array[j + 1].tempoChegada && array[j].status > array[j + 1].status)
            ) 
            // Troca os processos
            swap(&array[j], &array[j+1]);  
        }
    }
}

void printarTurnaround(Processo* arrayProcessos, int qntdProcessos, int* tempoOciosoCPU){

    bubbleSortPID(arrayProcessos, qntdProcessos);
    // Imprime o turnaround de cada processo
    printf("\n\n------------------------------------------------------------\n");
    printf("===== TURNAROND DOS PROCESSOS =====\n");
    printf("------------------------------------------------------------\n");

    for(int aux = 0; aux < qntdProcessos; aux++){

        printf("O turnaround do processo de PID %d foi: %d unidades de tempo.\n", 
            arrayProcessos[aux].PID, 
            arrayProcessos[aux].turnaround);
    }

    printf("============================================================");
    printf("\nA CPU ficou ociosa por %d unidades de tempo.\n", *tempoOciosoCPU);
    printf("============================================================");
}

int main() {
    int qntdProcessos = 5;
    srand(time(NULL)); // Semente para gerar números aleatórios diferentes a cada execução
    
    if (qntdProcessos > MAX_PROCESSOS) {
        printf("Erro: número de processos excede o limite (%d).\n", MAX_PROCESSOS);
        return 1;
    }

    // Criação dos processos
    Processo *arrayProcessos = lerProcessosArquivos("processos.txt", qntdProcessos);
    bubbleSortNovos(arrayProcessos, qntdProcessos); // Ordena por tempo de chegada

    // Exibição das informações iniciais
    printf("===== PROCESSOS CRIADOS =====\n");
    printf("PID | Chegada | Servico | I/O | Tempo I/O | Inicio I/O\n");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < qntdProcessos; i++) {
        printf(" %2d |   %3d    |   %3d   |  %c  |    %2d     |     %2d\n",
            arrayProcessos[i].PID,
            arrayProcessos[i].tempoChegada,
            arrayProcessos[i].tempoServico,
            arrayProcessos[i].necessidadeIO,
            arrayProcessos[i].tempoIO,
            arrayProcessos[i].tempoInicioIO);
    }

    printf("========================================================\n\n");

    // Inicializa as filas do escalonadores
    FilaProcessos *fp = (FilaProcessos*) malloc(sizeof(FilaProcessos));
    initFilaProcessos(fp);

    int tempoOciosoCPU = 0;
    // Chama o escalonador
    escalonador(fp, arrayProcessos, qntdProcessos, &tempoOciosoCPU);

    // Printa o turnaround
    printarTurnaround(arrayProcessos, qntdProcessos, &tempoOciosoCPU);

    // Libera memória
    liberarFilaProcessos(fp);
    free(arrayProcessos);

    return 0;
}
