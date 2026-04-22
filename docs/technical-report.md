# Relatorio Tecnico

## 1. Objetivo do projeto

O projeto implementa uma estacao IoT interativa baseada em FreeRTOS, com monitoramento ambiental, interface local, conectividade WiFi e integracao MQTT com o Adafruit IO. A solucao foi organizada em componentes para facilitar manutencao, reuso e evolucao.

## 2. Conceitos do curso aplicados

### FreeRTOS

- Uso de tarefas dedicadas para aquisicao de sensores, interface local e tratamento de entradas.
- Uso de fila para transportar eventos do teclado.
- Uso de mutex para proteger o estado compartilhado da interface e dos limites.

### Perifericos do microcontrolador

- ADC para leitura do LDR.
- GPIO para o DHT11 e teclado matricial.
- I2C para o display OLED SSD1306.
- PWM com LEDC para LED RGB e buzzer.
- UART para comandos e consulta dos logs.

### Conectividade

- WiFi em modo `STA` para conexao a rede local.
- MQTT sobre Adafruit IO para telemetria e acionamento remoto.
- SNTP para obter data e hora reais e enriquecer o log de alarmes.

### Persistencia

- NVS usada para salvar os limites configurados pelo usuario.
- NVS usada para guardar o historico textual de alarmes.

## 3. Arquitetura proposta

O `app_main.c` inicializa os modulos e cria as tarefas principais. A leitura dos sensores ocorre de forma periodica. Cada amostra e comparada com os limites configurados. Caso uma violacao seja detectada, o sistema:

1. Registra o evento em NVS.
2. Aciona o buzzer.
3. Atualiza o OLED.
4. Publica o estado do alarme via MQTT.

O LED RGB e tratado como um atuador remoto: os valores de vermelho, verde e azul chegam por feeds separados do Adafruit IO e sao aplicados por PWM.

## 4. Justificativas tecnicas

### Modularizacao em componentes

Separar WiFi, MQTT, sensores, display e armazenamento reduz acoplamento e facilita testes isolados. Essa estrutura tambem ajuda na avaliacao, porque deixa claro o papel de cada parte do sistema.

### Uso de filas e mutex

O teclado gera eventos assincronos. Uma fila evita polling misturado com logica de negocio. O mutex garante consistencia ao acessar limites, leituras e estado do alarme em tarefas diferentes.

### Log textual em NVS

Para um projeto academico, guardar o log como texto simplifica a inspecao via serial e reduz complexidade de serializacao. Em uma versao futura, o log pode virar uma estrutura circular binaria.

### Buzzer com desligamento manual

O alarme sonoro foi pensado para ficar latched durante o episodio de falha, exigindo acao humana para silenciamento. Isso atende ao requisito de seguranca operacional do enunciado.

## 5. Fluxo de funcionamento

1. O sistema sobe o NVS e carrega os limites salvos.
2. Inicia WiFi em modo Station.
3. Inicia sincronizacao SNTP.
4. Conecta ao Adafruit IO via MQTT.
5. Le periodicamente luminosidade, temperatura e umidade.
6. Publica os dados nos feeds.
7. Compara a leitura com os limites.
8. Caso haja violacao, registra o alarme, aciona buzzer e informa no OLED.
9. O usuario pode ajustar limites pelo teclado ou consultar logs pela serial.

## 6. Possiveis melhorias

- Persistir tambem o estado do LED RGB.
- Adicionar pagina extra no OLED para navegar por todo o historico de configuracoes.
- Publicar os limites atuais no Adafruit IO.
- Implementar filtro ou media movel no LDR para reduzir ruido.
- Trocar o log textual por buffer circular com indice.
- Adicionar watchdog de tarefas e reconexao MQTT mais elaborada.

## 7. Conclusao

O projeto atende os requisitos centrais do enunciado e demonstra a integracao entre RTOS, perifericos, rede e persistencia. A principal qualidade da solucao esta na organizacao modular, que facilita entendimento, manutencao e apresentacao academica.
