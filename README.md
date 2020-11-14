<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/2_logotipo_COLORIDO_horizontal.jpg" height="400" width="800" >

# Medição de temperatura, umidade e distância através de conexão WIFI com Socket TCP/IP
 
## Introdução

Este projeto tem como objetivo conectar o WEMOS D1 em uma rede Wi-Fi fixa e através de um socket TCP/IP, ler os sensores HC-SR04 (ultrassônico) e DHT11. As etapas de conexão Wi-Fi é sinalizado no LED e em caso de 5 tentativas de conexão falharem, pressionar um botão para reinicio do processo de conexão Wi-Fi.

## Conhecimentos

Este projeto ensina a manipular *Tasks*, *EventGroups*, *Queue*, sensores *DHT11* e *HC-SR04*, *socket*, *GPIOs* e *LOGs*.

## Lista de materiais

1. Wemos D1 com cabo usb micro - 1 peça 
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/wemos-d1-r2-99136.jpeg" height="250" width="400" >

2. Protoboard 830 pontos - 1 peça
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/protoboard.JPG" height="200" width="400" >

3. Sensor de temperatura DHT11 - 1 peça
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/dht11_pins.png" height="350" width="350" >

4. Sensor ultrassonico HC-SR04 - 1 peça
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/HC-SR04-PINOUT.jpg" height="248" width="400" >

5. Tacswitch 7 mm - 1 peças
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/tac.jpg" height="150" width="150" >

6. Jumpers coloridos macho/macho - 10 peças
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/jumpers%20protoboard.jpg" height="250" width="250" >

7. Dispositivo Android com o App "TCP Terminal" - <https://play.google.com/store/apps/details?id=com.hardcodedjoy.tcpterminal>
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/SmartPhone.JPG" height="200" width="400" >

## Tecnologias 
 
Aqui estão as tecnologias usadas neste projeto
 
* Chip ESP8266 com WIFI integrado
* APP TCP Terminal para Android
* Wemos D1 
* Exemplos ESP8266 para WIFi
* Exemplos ESP8266 para Socket TCP
* Terminal mingw32
* FreeRTOS
* Tasks
* EventGroups
* Sensores DHT11 e HC-SR04
 
## Esquema eletrônico:
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/esquema.jpg" height="200" width="400" >

## Ferramentas de compilação:

Siga o passo a passo descrito no site oficial do fabricante do chip ESP8266 para a instalação correta:

> *https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/*

E também o video explicativo *Instalação do RTOS-SDK (ESP8266)*:

> *https://www.youtube.com/watch?v=84tuQaV8N0g&feature=youtu.be*

## Serviços usados
 
* Github
 
## Iniciando o Projeto
 
### Bibliotecas:

* Copie os arquivos de dentro da pasta 'Libs' 
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/Libs.JPG" height="200" width="400" >

* E cole-os dentro do diretorio *"ESP8266_RTOS_SDK/components"*.
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/sdkComponentes.JPG" height="200" width="400" >

### Configurações Iniciais:

No Repositório Main, você encontrará o arquivo main.c
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/main.JPG" height="200" width="400" >

 Abra-o e edite as configurações conforme segue abaixo:

* configure o SSID e Senha do seu roteador:
    ```
    // Wi-fi -----------------------------------
    #define ESP_WIFI_SSID "Seu-Wi-FI" // CONFIG_ESP_WIFI_SSID
    #define ESP_WIFI_PASS "Sua Senha" // CONFIG_ESP_WIFI_PASSWORD
    ```

* Configure o IP do servidor e a porta:
    ```
    // Socket TCP ------------------------------
    #define CONFIG_EXAMPLE_IPV4
    #ifdef CONFIG_EXAMPLE_IPV4
    #define HOST_IP_ADDR "192.168.43.1"
    #else
    #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
    #endif

    #define PORT 5005
    ```

* To create the bank and do as migrations:
>    $ rake db:create db:migrate db:seed
* To run the project:
>    $ rails s
 
## How to use
 
Here will be the images and descriptions. Principal content.
 
 
## Features
 
  - Here will be the features.
 
 
## Links
 
  - Link of deployed application: (if has been deployed)
  - Repository: https://link_of_repository
    - In case of sensitive bugs like security vulnerabilities, please contact
      YOUR EMAIL directly instead of using issue tracker. We value your effort
      to improve the security and privacy of this project!
 
 
## Versioning
 
1.0.0.0
 
 
## Authors
 
* **YOUR NAME**: @YOUR_GITHUB_NICKNAME (https://github.com/YOUR_GITHUB_NICKNAME)
 


