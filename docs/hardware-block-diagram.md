# Diagrama de Blocos do Hardware

```mermaid
flowchart LR
    PSU["Fonte 5V / USB"] --> BOARD["Franzininho WiFi LAB01"]

    subgraph Inputs["Entradas"]
        LDR["LDR + divisor resistivo"]
        DHT["Sensor DHT11"]
        KEY["Teclado 4x4"]
    end

    subgraph Outputs["Saidas"]
        OLED["Display OLED SSD1306 I2C"]
        RGB["LED RGB PWM"]
        BUZ["Buzzer PWM"]
        UART["Serial USB"]
    end

    subgraph Cloud["Nuvem"]
        WIFI["Rede WiFi"]
        AIO["Adafruit IO / MQTT"]
    end

    LDR -->|"ADC"| BOARD
    DHT -->|"GPIO"| BOARD
    KEY -->|"Linhas e colunas GPIO"| BOARD

    BOARD -->|"I2C"| OLED
    BOARD -->|"PWM"| RGB
    BOARD -->|"PWM"| BUZ
    BOARD -->|"UART"| UART

    BOARD -->|"STA WiFi"| WIFI --> AIO
    AIO -->|"Comandos MQTT"| BOARD
```

## Leituras e comandos

- LDR envia luminosidade para o ADC.
- DHT11 envia temperatura e umidade por um pino digital.
- O teclado permite configurar limites e desligar o buzzer.
- O OLED mostra conectividade, leituras, alarmes e limites.
- O Adafruit IO recebe os dados e devolve os comandos do LED RGB.
