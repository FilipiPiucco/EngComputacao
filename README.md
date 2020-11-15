<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/IoT.jpg" height="550" width="900" >

# Medição de temperatura, umidade e distância através de conexão WIFI com Socket TCP/IP
 
## Introdução

Este projeto tem como objetivo conectar o WEMOS D1 em uma rede Wi-Fi fixa e através de um socket TCP/IP, ler os sensores HC-SR04 (ultrassônico) e DHT11. As etapas de conexão Wi-Fi é sinalizada no LED e em caso de 5 tentativas de conexão falharem, pressionar um botão para reinicio do processo de conexão Wi-Fi.

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
<img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/AppTCPTerminal.jpeg" height="450" width="270" >

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

* No Repositório Main, você encontrará o arquivo main.c
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

 ### Compilando
 
 * Abra o terminal "mingw32" e navegue até o diretório do projeto onde se encontra o arquivo *sdkconfig* *
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/SDKConfig.JPG" height="200" width="400" > 
 
 * Utilizando o comando:
 
 >  cd "C:\esp8266\ESP8266_RTOS_SDK\examples\IoTII\Trabalho3"
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/MINGW32.JPG" height="200" width="400" > 
 
 
 * Feito isto você estará dentro do repositório
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/MINGW32_2.JPG" height="200" width="400" > 
 
 
 * Conecte sua WEMOS D1 na porta USB de seu computador e verifique qual porta *COM* foi reservada:
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/PortaCom.JPG" height="200" width="400" >
 
 
 * Agora é a hora de realizar o *manuconfig* e configurar a porta COM verficada anteriormente. Para isso utiliza o comando:
 
 > make menuconfig
  <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/MenuConfig.JPG" height="200" width="400" >
  
 * Aplicado o comando a janela abaixo será exibida. Navegue até *Serial flasher config*:
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/MenuConfig_2.JPG" height="200" width="400" >
 
 
 * Depois *Default serial port* 
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/MenuConfig_3.JPG" height="200" width="400" >
 
 * Indique a porta COM
 <img src="https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/MenuConfig_4.JPG" height="200" width="400" >

 * De *Ok*, após realize um *Save* e por fim selecione *Exit* até finalizar o *Menu config* e retornar ao terminal MINGW32
 
 * De volta ao terminal MINGW32 execute o comando:
 
 > make build
 
 * Por fim execute para tranferir o código e abrir o monitor serial execute o comando:
 
 >  make flash monitor
 
 Nota: O camando *make flash monitor* poderá demorar alguns minutos quando executado da primeira vez. 
 
 * Finalizado a transferiencia do código é hora de testar!
 
 Nota: Para baixar o video de demosntração acesse:
 
 <https://github.com/FilipiPiucco/EngComputacao/blob/main/Public/ReadmeImagens/Demonstra%C3%A7%C3%A3o.mp4>


## Links
 
  - App TCP Terminal: <https://play.google.com/store/apps/details?id=com.hardcodedjoy.tcpterminal>
  - instalação ESP8266: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/
  - Instalação do RTOS-SDK (ESP8266): https://www.youtube.com/watch?v=84tuQaV8N0g&feature=youtu.be
 
## Versão
 
1.0.0.0
  
## Autores
 
* **Filipi Bitencourt Piucco**: @FilipiPiucco (https://github.com/FilipiPiucco)
* **Jonas Geremias**: @jonasgeremias (https://github.com/jonasgeremias)
 


