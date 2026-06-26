# Monitor Portátil de Frequência Cardíaca

<p align="center">
  <img src="docs/imagens/Foto Capa.png" alt="Banner do Projeto" width="100%">
</p>

## 📌 Descrição do Projeto

Este projeto consiste no desenvolvimento de um sistema embarcado portátil para monitoramento de sinais vitais, capaz de medir Frequência cardíaca (BPM – Beats Per Minute)

O sistema é baseado no microcontrolador **RP2040** e utiliza sensores ópticos para aquisição dos sinais biológicos através da técnica de **fotopletismografia (PPG)**, empregando LEDs nas faixas infravermelha e juntamente com um fotodetector.

O projeto foi desenvolvido para a disciplina de **Instrumentação e Microcontroladores e Sistemas Microcontrolados**, com foco na aplicação na área da saúde.

# 🎯 Objetivo

Desenvolver um sistema embarcado completo utilizando o microcontrolador RP2040, integrando:

- Construção e integração de sensor analógico;
- Condicionamento e tratamento de sinais biomédicos;
- Aquisição e processamento digital de sinais;
- Interface de visualização de dados;
- Acionamento de saídas/alertas;
- Desenvolvimento de PCB e estrutura mecânica.

O sistema busca fornecer medições básicas de sinais vitais de forma portátil, didática e de baixo custo.

# 🧠 Fundamentação Teórica

O projeto utiliza o princípio da **fotopletismografia (PPG)**, técnica óptica que detecta variações no volume sanguíneo através da absorção de luz pelos tecidos.

É utilizado o comprimento de onda infravermelho:
- 🔴 LED Infravermelho (~940 nm)

A hemoglobina oxigenada e desoxigenada absorvem essas frequências de forma diferente, permitindo perceber a variação da corrente sanguínea nos vasos.

Ou seja, as oscilações periódicas do sinal PPG permitem calcular a frequência cardíaca do usuário.

# ⚙️ Funcionalidades

- Medição da frequência cardíaca (BPM)
- Filtragem analógica e digital do sinal
- Exibição dos dados em display OLED
- Comunicação serial via USB/UART
- Sistema de alertas sonoros
- Exercicício de respiração com vibracall
- Estrutura mecânica impressa em 3D
- PCB dedicada para integração do sistema

# 🧩 Arquitetura do Sistema

## Diagrama de blocos

<p align="center">
  <img src="docs/imagens/Figura 2 - Diagrama de blocos da manipulação do sinal.png" alt="Tratamento das grandezas do sinal" width="100%">
</p>

## Fluxograma da lógica de programação
<p align="center">
  <img src="docs/imagens/Figura 10 – Fluxograma da lógica de programação.png" alt="Leitura no ADC, filtros de média móvel digitais, detecção de picos e cálculo do BPM" width="100%">
</p>

# 🔌 Hardware Utilizado

## Microcontrolador

- RP2040 (Raspberry Pi Pico)

## Sensores

- LEDs Infravermelho
- Fotodiodo/Fototransistor

## Condicionamento de Sinal

- Amplificador de Transimpedância
- Filtros passa-baixa
- Amplificador de Ganho

## Interface

- Display OLED I2C
- Comunicação Serial USB/UART
- Buzzer para alertas
- Vibracall para respiração

## Estrutura Mecânica

- Suporte para dedo/sensor em 3D

# 🖥️ Firmware

O firmware foi desenvolvido em:

- Linguagem C
- SDK oficial do RP2040
- Visual Studio Code

## Principais módulos

- Aquisição ADC
- Controle dos LEDs
- Filtragem digital
- Cálculo de BPM
- Estimativa de SpO₂
- Comunicação serial
- Interface gráfica

# 📂 Estrutura do Repositório

```text
├── firmware/
│   ├── src/
│   ├── include/
│   └── CMakeLists.txt
│
├── hardware/
│   ├── esquematico/
│   ├── pcb/
│   └── componentes/
│
├── mecanica/
│   ├── stl/
│   └── cad/
│
├── docs/
│   ├── relatorio/
│   ├── apresentacao/
│   └── imagens/
│
└── README.md
```

# 📊 Tratamento de Sinais

O sinal captado pelo fotodiodo apresenta baixa amplitude e elevada susceptibilidade a ruídos, exigindo técnicas de condicionamento e processamento.

Foram implementados:

- Amplificação analógica
- Filtragem passa-faixa
- Remoção de offset DC
- Média móvel
- Filtragem digital

O objetivo é melhorar a relação sinal-ruído e permitir medições mais confiáveis.

# 📸 Fotos do Projeto

## Protótipo

> Inserir imagem do protótipo aqui

<p align="center">
  <img src="docs/imagens/Figura 5 – Protótipo inicial analisado com osciloscópio.png" alt="Leitura no ADC, filtros de média móvel digitais, detecção de picos e cálculo do BPM" width="100%">
</p>

## Esquema elétrico

> Inserir imagem do esquemático aqui

<p align="center">
  <img src="docs/imagens/Figura 4 – Circuito analógico para tratamento do sinal.png" alt="Leitura no ADC, filtros de média móvel digitais, detecção de picos e cálculo do BPM" width="100%">
</p>


# PCB

> Inserir imagem da PCB aqui

<p align="center">
  <img src="docs/imagens/Figura 6 – PCI feita no EasyEDA, na esquerda, e ela após sair da CNC, na direita.png" alt="Leitura no ADC, filtros de média móvel digitais, detecção de picos e cálculo do BPM" width="100%">
</p>

# 🎥 Vídeo de Funcionamento

> Inserir link do vídeo demonstrativo

```text
youtube.com/(alguma coisa)
```

# 📄 Relatório Técnico

> O relatório técnico completo do projeto encontra-se em:

```text
/docs/relatorio/
```

# 👨‍💻 Integrantes

| Nome | RA |
|---|---|
| Erich Abreu Serafim | R.A. 23.10022-2 |
| João Pedro de Jesus Cândido Silva | R.A. 23.01416-4 |

# 🏫 Instituição

**Instituto Mauá de Tecnologia (IMT)**

**Disciplina:** Instrumentação e Microcontroladores e Sistemas Microcontrolados

**Professores:**

- Prof. Andressa Martins  
- Prof. Rodrigo França

# 📅 Cronograma

| Etapa | Data |
|---|---|
| Apresentação do Projeto | 23/06/2026 |
| Entrega do Relatório | 28/06/2026 |

# 📜 Licença

Copyright (c) 2026 Instituto Mauá de Tecnologia (IMT)

Este projeto foi desenvolvido para fins acadêmicos na disciplina de Instrumentação e Microcontroladores e Sistemas Microcontrolados.

Licenciado sob a licença MIT.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
