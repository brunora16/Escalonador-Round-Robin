# Escalonador de Processos Round Robin com Feedback

Este projeto simula um escalonador de processos usando a estratégia Round Robin com duas filas de prioridade (alta e baixa), aging (feedback), e suporte a operações de I/O. O objetivo é estudar políticas de escalonamento, preempção, aging e o impacto de operações de entrada/saída no tempo de turnaround dos processos.

## Funcionalidades

- **Escalonamento com duas filas de prioridade** (alta e baixa)
- **Quantum configurável** para execução dos processos
- **Aging (feedback):** processos promovidos para alta prioridade após muito tempo de espera
- **Simulação de operações de I/O** (Disco, Fita Magnética, Impressora)
- **Cálculo do turnaround** de cada processo e tempo ocioso da CPU
- **Leitura dos processos de um arquivo `processos.txt`** (gerado por um programa auxiliar)

## Estrutura dos arquivos

- `escalonador.c` — Código principal do simulador/escalonador
- `geradorProcessos.c` — Gera o arquivo `processos.txt` com os processos de entrada
- `processos.txt` — Arquivo de entrada com os processos a serem simulados

## Formato do arquivo `processos.txt`

Cada linha representa um processo com os seguintes campos:
```
PID tempoChegada prioridade status tempoServico feedback necessidadeIO tempoIO tempoInicioIO tempoEspera
```
Exemplo:
```
0 0 1 0 12 0 C 6 3 0
```

## Saída do programa

O programa exibe:
- A tabela dos processos criados
- O passo a passo da execução (filas, execuções, I/O, aging)
- O turnaround de cada processo
- O tempo total em que a CPU ficou ociosa

## Observações

- O número máximo de processos é definido por `MAX_PROCESSOS` em `escalonador.c`.
- O quantum de execução é definido pela constante `quantum`.
- O aging é controlado pela constante `LIMITE_ESPERA`.
- O código foi desenvolvido para fins didáticos e pode ser adaptado para outros experimentos de escalonamento.

---
