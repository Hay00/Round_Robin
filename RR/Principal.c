#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int processo[16][7];
/* [PID]  [DURAÇÃO] [INGRESSO] [RESTO DA DURAÇÃO] [ESTADO 0 = Finalizado, 1 = Ativo ] [Já foi escolhido na FILA] [ Tempo de espera]
     0        1         2              3                    4                                   5                       6
 */

int tempo = 0;
int controle = 0; // 0 = Nenhum processo entrou na fila | 1 = Já entrou.
int quant_processos, quantum, processos_restantes, INICIO, FINAL, checar_processo, processo_rodando;
int fila_processos[100], tempo_execucao[16], tempo_parou[16], tempo_inicioExec[16];
char lista_nomes[16];

void criar_processo(int quant) {
    for (int count = 0; count < quant; count++) {
        printf("\n");
        printf("Processo %d:", count + 1);

        //Define o PID pelo INDEX do vetor.
        processo[count][0] = count;

        //Nome respectivo do processo.
        printf("\nInsira o nome do processo (Max 1 letra!):");
        scanf("%s", &lista_nomes[count]);

        //Tempo do processo
        printf("Insira a duração do processo:");
        scanf("%d", &processo[count][1]);

        //Ingresso do processo.
        printf("Insira o ingresso do processo:");
        scanf("%d", &processo[count][2]);

        //Passa a duração do processo para o resto da duração.
        processo[count][3] = processo[count][1];


        //Estado do processo.
        processo[count][4] = 1;

        //Define se foi colocado na fila.
        processo[count][5] = 0;

        printf("Processo %d criado!\n\n", count + 1);
    }
}

int rodar_processo(int id_processo) {
    //Entra se o processo tiver mais resto que o quantum.
    tempo_inicioExec[id_processo] = tempo;
    int duracao = 1;
    int ingresso = 2;
    int resto = 3;
    int estado = 4;

    if (processo[id_processo][duracao] && processo[id_processo][resto] >= quantum) {
        int continuaExecucao;
        for (int count = 0; count < quantum; count++) {
            processo[id_processo][resto] -= 1;
            tempo++;
            if (processo[id_processo][resto] < 1) {
                processo[id_processo][estado] = 0;
                processos_restantes--;
                tempo_execucao[id_processo] += tempo - processo[id_processo][ingresso];
                continuaExecucao = 0;
                break;
            } else {
                continuaExecucao = 1;
            }
        }
        tempo_parou[id_processo] = tempo;
        return continuaExecucao;
    } else {
        int continuaExecucao;
        for (int count = 0; count < processo[id_processo][resto]; count++) {
            processo[id_processo][resto] -= 1;
            tempo++;
            if (processo[id_processo][resto] < 1) {
                processo[id_processo][estado] = 0;
                processos_restantes--;
                tempo_execucao[id_processo] += tempo - processo[id_processo][ingresso];
                continuaExecucao = 0;
                break;
            } else {
                continuaExecucao = 1;
            }
        }
        tempo_parou[id_processo] = tempo;
        return continuaExecucao;
    }
}



//Coloca os valores necessários para a lógica da fila funcionar.

void criar_fila() {
    INICIO = 1;
    FINAL = 0;
}



//Retorna o tamanho da fila atual.

int tamanho_fila() {
    return (FINAL - INICIO) + 1;
}



//Adiciona o PID do processo na fila.

void colocar_fila(int valor) {
    FINAL++;
    fila_processos[FINAL] = valor;
}

int olhar_fila() {
    return fila_processos[INICIO];
}



//Retira o PID do processo da fila.

int retirar_fila() {
    int valor = fila_processos[INICIO];
    INICIO++;
    return valor;
}

int main(int argc, char** argv) {
    //Inicia a fila dos processos.
    criar_fila();

    //Define a quantidade de processos que irão rodar.
    while (1) {
        printf("Insira a quantidade de processos (Max 16 processos):");
        scanf("%d", &quant_processos);
        if (quant_processos > 16) {
            printf("\n\n Passou do Limite!\n\n");
        } else {
            break;
        }
    }

    processos_restantes = quant_processos;

    //Chama a função que cria os processos.
    criar_processo(quant_processos);

    //Definição do quantum.
    printf("\nInsira o quantum desejado:");
    scanf("%d", &quantum);

    //Define o tempo no qual o processo parou com o seu ingresso.
    // e conta o tempo total dos processos.
    int max_tempo;
    for (int count = 0; count < quant_processos; count++) {
        if (quant_processos < 2) {
            max_tempo += processo[count][1] + processo[count][2];
        } else {
            tempo_parou[count] = processo[count][2];
            max_tempo += processo[count][1];
        }
    }

    int lista_tempo[quant_processos][max_tempo];
    /* Valores que definem o que o processo estava fazendo em determinado tempo:
     * -1 = NULL
     * 1 = Ingresso
     * 2 = Esperando
     * 3 = Rodando.
     */

    //Seta o ingresso e o resto para -1/NULL, para evitar escapes de tempo.
    for (int i = 0; i < quant_processos; i++) {
        for (int j = 0; j < max_tempo; j++) {
            if (processo[i][2] == j) {
                lista_tempo[i][j] = 1;
            } else {
                lista_tempo[i][j] = -1;
            }
        }
    }

    //Roda os processos.
    while (processos_restantes > 0) {
        //Incrementa o tempo de espera dos processos.
        for (int j = 0; j < quant_processos; j++) {
            if ((fila_processos[INICIO] == processo[j][0]) && (tamanho_fila() > 0)) {
                processo[j][6] += tempo - tempo_parou[j];
            }
        }

        //Coloca na fila o processo que ingressou no tempo atual.
        for (int i = 0; i < quant_processos; i++) {
            int estado = processo[i][5];
            int ingresso = processo[i][2];

            if ((ingresso <= tempo) && (estado == 0)) {
                colocar_fila(processo[i][0]);
                processo[i][5] = 1;
                controle = 1;

                /*Caso o processo entrar no meio do quantum inicial corrige
                 * o valor de espera que faltava, junto com a tabela de espera.
                 */
                if ((ingresso <= tempo - 1) && (tempo == quantum)) {
                    processo[i][6] += tempo - ingresso;
                    if (quantum >= 2) {
                        for (int j = ingresso + 1; j <= tempo; j++) {
                            lista_tempo[i][j] = 2;
                        }
                    }
                }
            }
        }

        //Se o processo terminou de executar, não volta pra fila.
        if (checar_processo == 1) {
            colocar_fila(processo_rodando);
        }

        //Roda o processo se algum já entrou na fila.
        if (controle == 1) {
            processo_rodando = retirar_fila();
            checar_processo = rodar_processo(processo_rodando);
        } else {
            tempo++;
        }

        //Passa os indices onde o processo foi executado para uma matriz.
        if ((tempo > processo[processo_rodando][2]) && (tempo_parou[processo_rodando] > processo[processo_rodando][2])) {
            for (int i = tempo_inicioExec[processo_rodando] + 1; i <= tempo; i++) {
                lista_tempo[processo_rodando][i] = 3;
            }
        }

        //Passa o tempo de espera do próximo processo para uma matriz.
        int prox_processo = olhar_fila();
        if ((tamanho_fila() != 0)) {
            for (int i = tempo_parou[prox_processo] + 1; i <= tempo; i++) {
                lista_tempo[prox_processo][i] = 2;
            }
        }
    }

    printf("\n---------------------------------------------------------------------------------------------------------");
    printf("\n ▓▓▓▓ : Executando");
    printf("\n ░░░░ : Esperando");
    printf("\n Ingr : Ingresso");
    printf("\n\nLista de Execução:");
    printf("\n  Nome:");
    for (int i = quant_processos - 1; i >= 0; i--) {
        printf("\n    %c: ", lista_nomes[i]);
        printf("   |");
        for (int j = 0; j < max_tempo; j++) {
            if (lista_tempo[i][j] == 3) {
                printf("▓▓▓▓");
            } else if (lista_tempo[i][j] == 2) {
                printf("░░░░");
            } else if (lista_tempo[i][j] == 1) {
                printf("Ingr");
            } else {
                printf("    ");
            }
            printf("|");
        }
    }

    printf("\n\n  Tempo:   ");
    for (int i = 0; i <= tempo; i++) {
        printf(" %d ", i);
        if (i > 9) {
            printf(" ");
        } else {
            printf("  ");
        }
    }
    printf("\n\n  Quantum : %d", quantum);

    printf("\n\n---------------------------------------------------------------------------------------------------------");
    printf("\n\nTabela dos Processos:");
    printf("\n  |PID|   |Nome|   |Duração|   |Ingresso|   |Tempo de execução|   |Tempo de espera|\n");

    //Mostra os valores respectivos dos processos.
    for (int i = 0; i < quant_processos; i++) {
        printf("   %d\t    %c\t     %d\t\t  %d\t\t    %d\t\t\t %d", processo[i][0], lista_nomes[i], processo[i][1], processo[i][2], tempo_execucao[i], processo[i][6]);
        printf("\n");
    }

    float tempoMedioExec;
    float tempoMedioEspera = 0;

    for (int count = 0; count < quant_processos; count++) {
        tempoMedioExec += tempo_execucao[count];
        tempoMedioEspera += processo[count][6];
    }

    //Tempos da execução.
    printf("\n---------------------------------------------------------------------------------------------------------");
    printf("\n\nTempo total de Execução: %d unidades de tempo.", tempo);
    printf("\nTempo médio de Execução: %2.2f unidades de tempo.", tempoMedioExec / quant_processos);
    printf("\nTempo médio de Espera: %2.2f unidades de tempo.", tempoMedioEspera / quant_processos);
    printf("\n\n---------------------------------------------------------------------------------------------------------");
    pause();
    return (EXIT_SUCCESS);
}