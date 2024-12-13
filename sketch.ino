#include <WiFi.h>
#include <HTTPClient.h>

// Definições dos pinos dos LEDs
#define led_azul 9
#define led_verde 41
#define led_vermelho 40
#define led_amarelo 42

// Definições dos pinos de entrada
const int botaoPin = 18;  // Pino do botão
const int ldrPin = 4;     // Pino do sensor LDR

// Limite para detecção de luz
int limiar = 600;

// Variáveis auxiliares
bool botaoPressionado = false;
bool estadoAnteriorBotao = LOW;
unsigned long ultimaMudancaEstado = 0;
const unsigned long intervaloDebounce = 50; // 50 ms para debounce

int contadorBotoes = 0; // Contador de pressionamentos do botão
unsigned long tempoUltimoPressionamento = 0;
const unsigned long intervaloResetContador = 5000; // 5 segundos para resetar o contador

void setup() {
  // Configuração dos pinos dos LEDs como saídas
  pinMode(led_azul, OUTPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  // Configuração inicial: todos os LEDs desligados
  digitalWrite(led_azul, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);

  // Inicialização do botão e do sensor LDR como entradas
  pinMode(botaoPin, INPUT);
  pinMode(ldrPin, INPUT);

  // Configuração serial para debug
  Serial.begin(9600);

  // Conexão WiFi
  WiFi.begin("Wokwi-GUEST", "");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi com sucesso!");
}

void loop() {
  // Leitura do estado do LDR
  int statusLDR = analogRead(ldrPin);

  // Leitura do botão com debounce
  bool leituraAtualBotao = digitalRead(botaoPin);
  unsigned long tempoAtual = millis();

  if (leituraAtualBotao != estadoAnteriorBotao) {
    ultimaMudancaEstado = tempoAtual; // Reinicia o tempo de debounce
  }

  if ((tempoAtual - ultimaMudancaEstado) > intervaloDebounce) {
    if (leituraAtualBotao == HIGH && !botaoPressionado) {
      botaoPressionado = true; // Marca que o botão foi pressionado
      Serial.println("Botão pressionado!");

      // Incrementa o contador e registra o tempo do pressionamento
      contadorBotoes++;
      tempoUltimoPressionamento = tempoAtual;

      Serial.print("Pressionamentos acumulados: ");
      Serial.println(contadorBotoes);
    }
  }

  estadoAnteriorBotao = leituraAtualBotao;

  // Reseta o contador se o botão não for pressionado por um intervalo maior que o definido
  if (tempoAtual - tempoUltimoPressionamento > intervaloResetContador) {
    contadorBotoes = 0;
  }

  // Verifica se o semáforo está no estado fechado e o botão foi pressionado 3 vezes
  if (contadorBotoes >= 3 && digitalRead(led_vermelho) == HIGH) {
    contadorBotoes = 0; // Reseta o contador após o envio do alerta

    // Envia a requisição HTTP para o alerta
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String enderecoServidor = "http://www.google.com.br/"; // URL do alerta (exemplo)

      http.begin(enderecoServidor.c_str());
      int codigoRespostaHTTP = http.GET();

      if (codigoRespostaHTTP > 0) {
        Serial.print("Código de resposta HTTP: ");
        Serial.println(codigoRespostaHTTP);
      } else {
        Serial.print("Erro na requisição HTTP, código: ");
        Serial.println(codigoRespostaHTTP);
      }
      http.end();
    } else {
      Serial.println("WiFi desconectado. Não foi possível enviar o alerta.");
    }
  }

  botaoPressionado = false; // Reseta o estado do botão após a leitura

  // Controle do semáforo
  if (statusLDR <= limiar) {
    // Modo noturno: piscar o LED amarelo
    digitalWrite(led_verde, LOW);
    digitalWrite(led_vermelho, LOW);
    digitalWrite(led_amarelo, HIGH);
    delay(500);
    digitalWrite(led_amarelo, LOW);
    delay(500);
  } else {
    // Modo convencional
    digitalWrite(led_verde, HIGH);
    delay(3000);
    digitalWrite(led_verde, LOW);
    digitalWrite(led_amarelo, HIGH);
    delay(2000);
    digitalWrite(led_amarelo, LOW);
    digitalWrite(led_vermelho, HIGH);
    delay(5000);
  }
}
