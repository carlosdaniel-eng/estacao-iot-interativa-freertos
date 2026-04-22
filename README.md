# Estacao IoT Interativa com FreeRTOS e MQTT

Projeto-base em ESP-IDF para a Franzininho WiFi LAB01, organizado em componentes reutilizaveis para sensores, atuadores, WiFi, MQTT, interface local e persistencia em NVS.

## O que o projeto entrega

- Leitura periodica de luminosidade via LDR e temperatura/umidade via DHT11
- Publicacao MQTT no Adafruit IO para `temperatura`, `umidade`, `luminosidade` e `alarmes`
- Controle remoto do LED RGB por feeds MQTT `led-red`, `led-green` e `led-blue`
- Acionamento automatico do buzzer quando algum limite configurado e ultrapassado
- Desligamento manual do buzzer pelo teclado
- Exibicao de sensores, conectividade e limites no display OLED SSD1306
- Ajuste local dos limites de alarme usando teclado matricial 4x4
- Registro de alarmes em NVS com timestamp sincronizado via SNTP
- Consulta e limpeza de logs pela serial
- Operacao apenas em modo Station com reconexao automatica

## Estrutura

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

## Mapeamento dos componentes

- `wifi_manager`: conexao em STA e reconexao automatica
- `mqtt_service`: conexao com Adafruit IO, publicacao dos sensores e assinatura dos feeds do LED RGB
- `sensor_service`: leitura de LDR via ADC e DHT11 via GPIO
- `actuator_service`: PWM do LED RGB e do buzzer
- `display_service`: driver SSD1306 via I2C e renderizacao da interface local
- `keypad_service`: varredura do teclado 4x4 e envio de teclas por fila
- `alarm_service`: comparacao das leituras com os limites configurados
- `storage_service`: persistencia dos limites e do log de alarmes em NVS
- `console_service`: comandos seriais `logs`, `clearlogs` e `limits`
- `time_service`: sincronizacao SNTP para carimbar os logs com data e hora

## Controles no teclado

- `A`: campo anterior
- `B`: proximo campo
- `C`: decrementa o limite selecionado
- `D`: incrementa o limite selecionado
- `*`: desliga o buzzer manualmente
- `#`: salva novamente os limites atuais

## Feeds esperados no Adafruit IO

- `temperatura`
- `umidade`
- `luminosidade`
- `alarmes`
- `led-red`
- `led-green`
- `led-blue`

## Como configurar

1. Instale o ESP-IDF 5.x.
2. Abra a pasta `iot-station-freertos`.
3. Ajuste WiFi, usuario e chave do Adafruit IO em [components/app_config/include/app_config.h](/C:/Users/danie/OneDrive/Documentos/New%20project/iot-station-freertos/components/app_config/include/app_config.h:1).
4. Revise os pinos do OLED, DHT11, LDR, teclado, buzzer e LED RGB no mesmo arquivo para bater com a sua montagem real.
5. Crie os feeds listados acima no Adafruit IO.

## Como compilar e gravar

```bash
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

## Uso pela serial

No monitor serial, digite:

- `logs` para listar o historico de alarmes
- `clearlogs` para apagar o historico
- `limits` para imprimir os limites atuais

## Observacoes importantes

- Os pinos em `app_config.h` sao uma base de referencia e devem ser adaptados ao hardware real montado no laboratorio.
- O projeto foi estruturado para estudo e extensao. Em uma entrega final, vale complementar com fotos, video e um diagrama feito no Draw.io ou Fritzing.
- O timestamp depende de WiFi e sincronizacao SNTP. Antes disso, o log usa `UNSYNCED`.

## Documentacao complementar

- Diagrama de blocos: [docs/hardware-block-diagram.md](/C:/Users/danie/OneDrive/Documentos/New%20project/iot-station-freertos/docs/hardware-block-diagram.md:1)
- Relatorio tecnico: [docs/technical-report.md](/C:/Users/danie/OneDrive/Documentos/New%20project/iot-station-freertos/docs/technical-report.md:1)
