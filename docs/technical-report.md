# Relatorio Tecnico

## 1. Apresentacao

O projeto consiste em uma estacao IoT interativa desenvolvida para a Franzininho WiFi LAB01. A proposta foi reunir em um unico sistema os principais conteudos trabalhados no curso, incluindo tarefas com FreeRTOS, leitura de sensores, acionamento de atuadores, comunicacao WiFi, uso de MQTT e armazenamento de dados em memoria nao volatil.

## 2. Objetivo

O objetivo principal foi montar uma aplicacao capaz de monitorar temperatura, umidade e luminosidade, publicar essas informacoes no Adafruit IO e reagir localmente quando os valores saem dos limites definidos pelo usuario.

Tambem foi considerado importante permitir configuracao diretamente no dispositivo, sem depender apenas da interface remota. Por isso, o teclado e o display OLED foram usados como interface local de operacao.

## 3. Recursos utilizados

### Hardware

- Franzininho WiFi LAB01
- Sensor DHT11
- LDR
- Display OLED SSD1306
- Teclado matricial 4x4
- LED RGB
- Buzzer

### Software e conceitos

- ESP-IDF
- FreeRTOS
- MQTT
- Adafruit IO
- NVS
- SNTP
- PWM, GPIO, ADC e I2C

## 4. Estrutura do software

O codigo foi dividido em componentes para evitar concentrar toda a logica em um unico arquivo. Essa separacao deixou o projeto mais legivel e facilitou o reuso das partes principais.

Cada modulo tem uma responsabilidade especifica:

- WiFi
- MQTT
- sensores
- display
- teclado
- atuadores
- alarmes
- armazenamento
- tempo
- console serial

No arquivo principal `app_main.c`, os modulos sao inicializados e as tarefas sao criadas. A partir disso, o sistema passa a executar a leitura periodica dos sensores, atualizar a interface local, receber entradas do teclado e publicar informacoes no broker MQTT.

## 5. Aplicacao dos conceitos do curso

### Uso do FreeRTOS

O projeto utiliza tarefas separadas para organizacao das funcoes principais. Essa divisao ajuda a manter a leitura dos sensores, a atualizacao do display e o tratamento do teclado funcionando de forma concorrente.

Tambem foram utilizados:

- fila para transportar as teclas lidas pelo teclado matricial
- mutex para proteger o estado compartilhado entre tarefas

Esses recursos foram importantes para evitar mistura entre leitura de entrada, atualizacao de interface e logica de alarme.

### Leitura de sensores

O DHT11 foi usado para temperatura e umidade, enquanto o LDR foi ligado ao ADC para medicao de luminosidade. As leituras sao feitas periodicamente e armazenadas em uma estrutura comum, o que facilita a comparacao com os limites e o envio por MQTT.

### Comunicacao WiFi e MQTT

O sistema opera somente em modo Station. Depois de conectar na rede, ele inicia a comunicacao MQTT com o Adafruit IO. Os dados de temperatura, umidade e luminosidade sao publicados em feeds separados.

O LED RGB tambem foi integrado ao Adafruit IO. Assim, o usuario pode alterar os valores dos canais vermelho, verde e azul de forma remota.

### Interface local

O display OLED mostra informacoes de conectividade, leituras e alarmes. O teclado permite navegar pelos campos e alterar os valores minimos e maximos configurados para cada variavel monitorada.

Essa parte foi importante porque tornou o sistema utilizavel mesmo sem depender diretamente do computador.

### Alarmes e persistencia

Quando uma leitura ultrapassa os limites configurados, o buzzer e acionado e um registro textual e salvo em NVS. O log guarda data, hora e os valores lidos no momento do evento.

Foi escolhida uma abordagem textual simples para os logs por ser suficiente para a proposta do projeto e por facilitar a consulta pela serial.

## 6. Fluxo geral do sistema

1. O NVS e inicializado.
2. Os limites salvos anteriormente sao carregados.
3. O WiFi conecta em modo Station.
4. O horario e sincronizado por SNTP.
5. O cliente MQTT conecta ao Adafruit IO.
6. Os sensores sao lidos periodicamente.
7. Os valores sao comparados com os limites configurados.
8. Se houver violacao, o alarme e disparado e o evento e salvo.
9. Os dados tambem sao enviados para os feeds MQTT.

## 7. Justificativas de implementacao

- A modularizacao foi escolhida para facilitar entendimento e manutencao.
- O uso de fila e mutex foi adotado para organizar a concorrencia entre tarefas.
- O buzzer permanece ativo ate o desligamento manual para deixar o alarme mais evidente.
- O uso de NVS permitiu manter configuracoes e historico mesmo apos reinicializacao.

## 8. Melhorias futuras

- adicionar filtro nas leituras do LDR
- publicar tambem os limites configurados no Adafruit IO
- melhorar a navegacao do display com mais telas
- usar uma estrutura circular para os logs
- adicionar tratamento mais completo para falhas de sensores

## 9. Conclusao

O projeto atende a proposta de integrar sensores, atuadores, conectividade e interface local em uma aplicacao unica. Alem disso, a divisao em componentes ajudou a aplicar a ideia de codigo reutilizavel, que era um dos focos da atividade.
