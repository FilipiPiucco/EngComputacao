
/*
Ex04 - TCP Cliente - Wi-Fi - GPIO - Sensores DHT11 e ultrassônico
Jonas P. Geremias
Filipi Bitencourt Piucco

Questão:1
Integrar o projeto de WiFi com GPIO com o projeto de Socket TCP, entretanto, utilizar do exemplo de Socket TCP server. 
O objetivo final é ter um sistema que consiga monitorar o status do WiFi do ESP8266 através de um LED, utilizar um 
botão para tentar nova conexão de WiFi e, quando conectado, criar um servidor Socket TCP para que algum cliente 
(App de celular, por exemplo) possa realizar a conexão e requisitar a informação dos sensores.
Enviar uma pasta compactada com o vídeo demonstrativo do exemplo e com o link do repositório no github. 
*/

// Bibliotecas ----------------------------------------------------------------
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/gpio.h"

/*	Relação entre pinos da WeMos D1 R2 e GPIOs do ESP8266
	Pinos-WeMos		Função			Pino-ESP-8266
		TX			TXD				TXD/GPIO1
		RX			RXD				RXD/GPIO3
		D0			IO				GPIO16	
		D1			IO, SCL			GPIO5
		D2			IO, SDA			GPIO4
		D3			IO, 10k PU		GPIO0
		D4			IO, 10k PU, LED GPIO2
		D5			IO, SCK			GPIO14
		D6			IO, MISO		GPIO12
		D7			IO, MOSI		GPIO13
		D8			IO, 10k PD, SS	GPIO15
		A0			Inp. AN 3,3Vmax	A0
*/

// Definições do hardware -----------------------------------------------------
#define LED_STATUS 2
#define LED_2 13
#define BUTTON 0

// Definições para o sensor DHT11 ----------
#include <dht.h>
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
#define DHT11_GPIO 16

// Sensor ultrassonico ---------------------
#include <ultrasonic.h>
#define MAX_DISTANCE_CM 500 // 5m max
#define TRIGGER_GPIO 4
#define ECHO_GPIO 5

// Wi-fi -----------------------------------
#define ESP_WIFI_SSID "Seu-Wi-FI" // CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS "Sua Senha" // CONFIG_ESP_WIFI_PASSWORD
#define ESP_MAXIMUM_RETRY 5      // CONFIG_ESP_MAXIMUM_RETRY
static int s_retry_num = 0;      // Quantidade de tentativas de conectar ao roteador
typedef enum led_status_wifi_t
{
    LED_STATUS_FAIL = 0,
    LED_STATUS_CONECTANDO,
    LED_STATUS_CONECTADO
} led_status_wifi_t;

// Socket TCP ------------------------------
#define CONFIG_EXAMPLE_IPV4
#ifdef CONFIG_EXAMPLE_IPV4
#define HOST_IP_ADDR "192.168.43.1"
#else
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#endif

#define PORT 5005

// Variáveis e definições globais ---------------------------------------------
QueueHandle_t tcp_queue;        // Fila de entrada de dados da task TCP
QueueHandle_t dht_queue;        // Fila de entrada de dados da task DHT
QueueHandle_t ultrasonic_queue; // Fila de entrada de dados da task Ultrasonic

static EventGroupHandle_t app_event_group;
#define WIFI_INIT_BIT BIT0
#define WIFI_CONNECTING_BIT BIT1
#define WIFI_CONNECTED_BIT BIT2
#define WIFI_IP_STA_GOT_IP BIT3
#define WIFI_FAIL_BIT BIT4

// LOG da aplicação -----------------------------------------------------------
#define DEBUG 1
#ifdef DEBUG
#include "esp_log.h"
static const char *LOG_WIFI = "wifi station";
static const char *LOG_APP = "APP";
static const char *LOG_TCP = "TCP";
static const char *LOG_DHT = "DHT";
static const char *LOG_ULTRASONIC = "ULTRASONIC";
static const char *LOG_MONITORA_GPIO = "MONITORA GPIO";
#endif

// Protótipos das funções -----------------------------------------------------
static void ultrasonic_task(void *pvParamters);
static void dht_task(void *pvParameters);
static void wifi_init_sta(void);
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void task_GPIO_Control(void *pvParameter);
static void task_tcp_client(void *pvParameters);
static void task_monitora_botao(void *pvParameter);

// Aplicação ------------------------------------------------------------------
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(LOG_WIFI, "Wi-Fi start");
        // Limpa os Bits ao iniciar
        xEventGroupClearBits(app_event_group, WIFI_CONNECTED_BIT | WIFI_IP_STA_GOT_IP | WIFI_FAIL_BIT);
        xEventGroupSetBits(app_event_group, WIFI_CONNECTING_BIT);
        s_retry_num = 0;
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(LOG_WIFI, "connect to the AP fail");
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            s_retry_num++;
            ESP_LOGI(LOG_WIFI, "retry to connect to the AP - retry %u ", s_retry_num);
            esp_wifi_connect();
        }
        else
        {
            xEventGroupSetBits(app_event_group, WIFI_FAIL_BIT);
            xEventGroupClearBits(app_event_group, WIFI_CONNECTED_BIT | WIFI_IP_STA_GOT_IP | WIFI_CONNECTING_BIT);
            ESP_LOGE(LOG_WIFI, "Erro ao conectar. Pressione o botão para tentar novamente.");
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(LOG_WIFI, "WIFI_EVENT_STA_CONNECTED");
        xEventGroupSetBits(app_event_group, WIFI_CONNECTED_BIT);
        xEventGroupClearBits(app_event_group, WIFI_CONNECTING_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(LOG_WIFI, "Recebeu IP:%s", ip4addr_ntoa(&event->ip_info.ip));

        xEventGroupSetBits(app_event_group, WIFI_IP_STA_GOT_IP);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        xEventGroupClearBits(app_event_group, WIFI_IP_STA_GOT_IP);
        esp_wifi_disconnect();
    }
}

static void task_GPIO_Control(void *pvParameter)
{
    gpio_set_direction(LED_STATUS, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_STATUS, 0);

    led_status_wifi_t modo_led_status = LED_STATUS_FAIL;

    int tempo_led_status = 0;
    int timeout = 0;
    bool status = 0;

    while (1)
    {
        EventBits_t event_bits = xEventGroupGetBits(app_event_group);

        // Analisa o modo do LED
        if (event_bits & WIFI_FAIL_BIT)
        {
            modo_led_status = LED_STATUS_FAIL;
            tempo_led_status = 2;
        }
        else if (event_bits & (WIFI_CONNECTED_BIT | WIFI_IP_STA_GOT_IP))
        {
            modo_led_status = LED_STATUS_CONECTADO;
        }
        else if (event_bits & WIFI_CONNECTING_BIT)
        {
            modo_led_status = LED_STATUS_CONECTANDO;
            tempo_led_status = 10;
        }
        else // Só por garantia
        {
            modo_led_status = LED_STATUS_FAIL;
            tempo_led_status = 2;
        }

        if (modo_led_status == LED_STATUS_CONECTADO)
            gpio_set_level(LED_STATUS, 0); // Led liga com  0
        else
        {
            if (timeout >= tempo_led_status)
            {
                timeout = 0;
                status ^= 1;
                gpio_set_level(LED_STATUS, status);
            }
            timeout++;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS); // base de tempo para a piscada
    }
}

static void task_monitora_botao(void *pvParameter)
{
    ESP_LOGI(LOG_MONITORA_GPIO, "Init");

    bool aux = 0;

    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);

    gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_2, 0);

    while (1)
    {
        // Aguarda o evento de erro e wifi ja iniciado
        xEventGroupWaitBits(app_event_group,
                            WIFI_FAIL_BIT,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);

        // Não ha necessidade de trava
        bool status_button = !gpio_get_level(BUTTON);
        if ((status_button) && (!aux))
        {
            ESP_LOGI(LOG_MONITORA_GPIO, "click");
            aux = 1;
            // Pisca led rapidamente 3 vezes
            for (int i = 1; i < 7; i++)
            {
                gpio_set_level(LED_2, i % 2); // Par ou impar
                vTaskDelay(50 / portTICK_RATE_MS);
            }

            // Sinaliza que vai conectar e manda wifi conectar
            s_retry_num = 0;
            xEventGroupSetBits(app_event_group, WIFI_CONNECTING_BIT);
            xEventGroupClearBits(app_event_group, WIFI_IP_STA_GOT_IP);
            xEventGroupClearBits(app_event_group, WIFI_CONNECTED_BIT);
            xEventGroupClearBits(app_event_group, WIFI_FAIL_BIT); // limpa este bit para que este serviço seja finalizado imediatamente
            esp_wifi_connect();
        }
        else if (!status_button)
            aux = 0;

        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

static void task_tcp_client(void *pvParameters)
{
    ESP_LOGI(LOG_TCP, "task_tcp_client init");
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
    static const char *msg = "Message from ESP8266 \r\n";
    char payload[128];

    while (1)
    {
        xEventGroupWaitBits(app_event_group,
                            (WIFI_CONNECTED_BIT | WIFI_IP_STA_GOT_IP),
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);

#ifdef CONFIG_EXAMPLE_IPV4
        ESP_LOGI(LOG_TCP, "Conectando no IP:%s", HOST_IP_ADDR);
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
        struct sockaddr_in6 destAddr;
        inet6_aton(HOST_IP_ADDR, &destAddr.sin6_addr);
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(PORT);
        destAddr.sin6_scope_id = tcpip_adapter_get_netif_index(TCPIP_ADAPTER_IF_STA);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(LOG_TCP, "Unable to create socket: errno %d", errno);
            vTaskDelay(3000 / portTICK_PERIOD_MS); // Aguarda 3 segundos e tenta novamente
            continue;
        }

        ESP_LOGI(LOG_TCP, "Socket created");

        int err = connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0)
        {
            ESP_LOGE(LOG_TCP, "Socket unable to connect: errno %d", errno);
            vTaskDelay(3000 / portTICK_PERIOD_MS); // Aguarda 3 segundos e tenta novamente
            close(sock);
            continue;
        }
        ESP_LOGI(LOG_TCP, "Successfully connected");

        while (1)
        {
            // Envia os dados das filas ou a mensagem padrão.
            if (xQueueReceive(tcp_queue, &rx_buffer, pdMS_TO_TICKS(1000)) == true)
            {
                sprintf(payload, "%s", rx_buffer);
                ESP_LOGE(LOG_TCP, "-> payload: %s", payload);
            }
            else
            {
                sprintf(payload, "%s", msg);
                ESP_LOGE(LOG_TCP, "-> data: %s", msg);
            }

            int err = send(sock, payload, strlen(payload), 0);
            if (err < 0)
            {
                ESP_LOGE(LOG_TCP, "Error occured during sending: errno %d", errno);
                break;
            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
            if (len < 0)
            {
                ESP_LOGE(LOG_TCP, "recv failed: errno %d", errno);
                break;
            }

            // Data received
            else
            {
                rx_buffer[len] = 0;
                ESP_LOGI(LOG_TCP, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(LOG_TCP, "%s", rx_buffer);
                uint32_t req = 0;

                if (strstr((const char *)rx_buffer, "DHT") != NULL)
                {
                    req = 10;
                    xQueueSend(dht_queue, &req, 0); // 10 = Valor aleatório apenas para notificar a task dht para enviar dados
                }

                if (strstr((const char *)rx_buffer, "ULTRA") != NULL)
                {
                    req = 10;
                    xQueueSend(ultrasonic_queue, &req, 0); // 10 = Valor aleatório apenas para notificar a task ultrasonic para enviar dados
                }
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(LOG_TCP, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    vTaskDelete(NULL);
}

static void wifi_init_sta(void)
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS},
    };

    if (strlen((char *)wifi_config.sta.password))
    {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);

    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupSetBits(app_event_group, WIFI_INIT_BIT); // Iniciou

    ESP_LOGI(LOG_WIFI, "wifi_init_sta Init.");
}

static void ultrasonic_task(void *pvParamters)
{
    ESP_LOGI(LOG_ULTRASONIC, "Init");
    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO};

    ultrasonic_init(&sensor);

    // Estabilizar o sensor após ligar o pullup
    vTaskDelay(50 / portTICK_PERIOD_MS);
    uint32_t req = 0;
    char res[128];

    while (true)
    {
        if (xQueueReceive(ultrasonic_queue, &req, pdMS_TO_TICKS(1000)) == true)
        {
            // Envia os dados das filas ou a mensagem padrão.
            uint32_t distance;
            esp_err_t ret = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
            if (ret != ESP_OK)
            {
                switch (ret)
                {
                case ESP_ERR_ULTRASONIC_PING:
                    sprintf(res, "%s", "Cannot ping (device is in invalid state)\r\n");
                    ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                    xQueueSend(ultrasonic_queue, &res, 0); // 10 = Valor aleatório apenas para notificar a task ultrasonic para enviar dados
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    sprintf(res, "%s", "Ping timeout (no device found)\r\n");
                    ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                    xQueueSend(ultrasonic_queue, &res, 0); // 10 = Valor aleatório apenas para notificar a task ultrasonic para enviar dados
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                    sprintf(res, "%s", "Echo timeout (i.e. distance too big)\r\n");
                    ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                    xQueueSend(ultrasonic_queue, &res, 0); // 10 = Valor aleatório apenas para notificar a task ultrasonic para enviar dados
                    break;
                default:
                    sprintf(res, "Erro não tratado:%u\r\n", ret);
                    ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                    xQueueSend(ultrasonic_queue, &res, 0);
                }
            }
            else
            {
                sprintf(res, "Distance: %d cm\r\n", distance);
                ESP_LOGE(LOG_ULTRASONIC, "%s", res);
                xQueueSend(tcp_queue, &res, 0);
            }
        }

        //ESP_LOGI(LOG_ULTRASONIC, "loop");

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

static void dht_task(void *pvParameters)
{
    ESP_LOGI(LOG_DHT, "Init");
    int16_t temperature = 0;
    int16_t humidity = 0;

    // DHT sensors that come mounted on a PCB generally have
    // pull-up resistors on the data pin.  It is recommended
    // to provide an external pull-up resistor otherwise...
    gpio_set_pull_mode(DHT11_GPIO, GPIO_PULLUP_ONLY);

    // Estabilizar o sensor após ligar o pullup
    vTaskDelay(50 / portTICK_PERIOD_MS);

    uint32_t req = 0;
    char res[128];

    while (1)
    {
        if (xQueueReceive(dht_queue, &req, pdMS_TO_TICKS(1000)) == true)
        {
            if (dht_read_data(sensor_type, DHT11_GPIO, &humidity, &temperature) == ESP_OK)
            {
                sprintf(res, "Humidity: %d%% Temp: %d°C\r\n", humidity / 10, temperature / 10);
                ESP_LOGI(LOG_DHT, "%s", res);
                xQueueSend(tcp_queue, &res, 0);
            }
            else
            {
                sprintf(res, "Could not read data from sensor dht\r\n");
                ESP_LOGE(LOG_DHT, "%s", res);
                xQueueSend(tcp_queue, &res, 0);
            }

            // 2000 porque deve estabilizar o componente, leituras mais rápidas esquentam o componente e erra a leitura de temperatura.
            vTaskDelay(1500 / portTICK_PERIOD_MS);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(LOG_APP, "Init app.");

    app_event_group = xEventGroupCreate();
    tcp_queue = xQueueCreate(10, sizeof(char) * 150);      // Cria a queue *buffer* com 10 slots de 150 caracteres
    dht_queue = xQueueCreate(10, sizeof(uint32_t));        // Cria a queue *buffer* com 10 slots de 4 Bytes
    ultrasonic_queue = xQueueCreate(10, sizeof(uint32_t)); // Cria a queue *buffer* com 10 slots de 4 Bytes

    // Serviços
    xTaskCreate(&dht_task, "dht_task", configMINIMAL_STACK_SIZE * 2, NULL, 5, NULL);
    xTaskCreate(&ultrasonic_task, "ultrasonic_task", configMINIMAL_STACK_SIZE * 2, NULL, 5, NULL);
    xTaskCreate(&task_tcp_client, "task_tcp_client", configMINIMAL_STACK_SIZE * 2, NULL, 1, NULL);
    xTaskCreate(&task_GPIO_Control, "task_GPIO_Control", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(&task_monitora_botao, "task_monitora_botao", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    wifi_init_sta();
}
