# Estacao IoT Interativa com FreeRTOS e MQTT

Este repositorio apresenta o desenvolvimento de uma estacao IoT para a placa Franzininho WiFi LAB01. O projeto foi organizado em modulos separados para facilitar a manutencao do codigo e aplicar, na pratica, os conteudos estudados sobre FreeRTOS, comunicacao MQTT, sensores, atuadores e persistencia de dados.

## Funcionalidades implementadas

- Leitura periodica de temperatura e umidade com o DHT11
- Leitura de luminosidade por meio de um LDR
- Publicacao dos dados no Adafruit IO via MQTT
- Controle remoto do LED RGB por feeds MQTT
- Acionamento automatico do buzzer quando algum limite configurado e ultrapassado
- Desligamento manual do buzzer pelo teclado matricial
- Exibicao das informacoes principais no display OLED
- Configuracao local de limites minimos e maximos para temperatura, luminosidade e umidade
- Registro de alarmes em NVS
- Consulta e limpeza do historico de alarmes pela serial
- Operacao em modo WiFi Station com tentativa de reconexao automatica

## Organizacao do projeto

```text
iot-station-freertos/
|-- CMakeLists.txt
|-- README.md
|-- components/
|   |-- actuator_service/
|   |-- alarm_service/
|   |-- app_common/
|   |-- app_config/
|   |-- console_service/
|   |-- display_service/
|   |-- keypad_service/
|   |-- mqtt_service/
|   |-- sensor_service/
|   |-- storage_service/
|   |-- time_service/
|   `-- wifi_manager/
|-- docs/
|   |-- hardware-block-diagram.md
|   `-- technical-report.md
`-- main/
    |-- CMakeLists.txt
    `-- app_main.c
```

## Descricao dos modulos

- `wifi_manager`: faz a conexao WiFi em modo STA e cuida da reconexao
- `mqtt_service`: publica sensores no Adafruit IO e recebe comandos para o LED RGB
- `sensor_service`: concentra a leitura do DHT11 e do LDR
- `actuator_service`: controla LED RGB e buzzer por PWM
- `display_service`: envia para o OLED os dados de sensores, alarmes e conectividade
- `keypad_service`: faz a leitura do teclado matricial
- `alarm_service`: compara as leituras com os limites definidos
- `storage_service`: salva limites e logs de alarmes em NVS
- `console_service`: trata os comandos enviados pela serial
- `time_service`: sincroniza data e hora via SNTP

## Controles do teclado

- `A`: seleciona o campo anterior
- `B`: seleciona o proximo campo
- `C`: diminui o valor do limite atual
- `D`: aumenta o valor do limite atual
- `*`: desliga o buzzer manualmente
- `#`: salva os limites

## Feeds utilizados no Adafruit IO

- `temperatura`
- `umidade`
- `luminosidade`
- `alarmes`
- `led-red`
- `led-green`
- `led-blue`

## Como configurar

1. Instale o ESP-IDF 5.x.
2. Abra a pasta do projeto `iot-station-freertos`.
3. Edite o arquivo `components/app_config/include/app_config.h`.
4. Configure:
   - nome e senha da rede WiFi
   - usuario e chave do Adafruit IO
   - pinos usados no OLED, DHT11, LDR, teclado, buzzer e LED RGB
5. Crie os feeds listados acima no Adafruit IO.

## Compilacao e gravacao

```bash
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

## Comandos disponiveis na serial

- `logs`: mostra o historico de alarmes salvo em memoria
- `clearlogs`: apaga o historico
- `limits`: exibe os limites atualmente configurados

## Observacoes

- Os pinos definidos em `app_config.h` devem ser ajustados de acordo com a montagem real do circuito.
- O registro com data e hora depende da sincronizacao SNTP.
- Antes da sincronizacao, o sistema usa `UNSYNCED` como referencia no log.

## Documentacao complementar

- Diagrama de blocos: [docs/hardware-block-diagram.md](docs/hardware-block-diagram.md)
- Relatorio tecnico: [docs/technical-report.md](docs/technical-report.md)
