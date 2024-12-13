#include <WiFi.h>
#include <HTTPClient.h>

// Pinos dos LEDs
#define led_azul 9
#define led_verde 41
#define led_vermelho 40
#define led_amarelo 42

// Pinos dos sensores e botões
const int botaoPin = 18;  // Pino do botão
const int ldrPin = 4;     // Pino do sensor LDR

// Limiar para detecção de luz
int limiar = 600;

// Variáveis de controle
bool botaoPressionado = false;
bool estadoAnteriorBotao = LOW;
unsigned long ultimaMudancaEstado = 0;
const unsigned long intervaloDebounce = 50; // 50 ms para debounce

int contadorBotoes = 0; // Contador de vezes que o botão foi pressionado
unsigned long tempoUltimoPressionamento = 0;
const unsigned long intervaloResetContador = 5000; // Reseta o contador depois de 5 segundos

void setup() {
  // Configura os LEDs como saída
  pinMode(led_azul, OUTPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  // No começo, desliga todos os LEDs
  digitalWrite(led_azul, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);

  // Configura o botão e o LDR como entrada
  pinMode(botaoPin, INPUT);
  pinMode(ldrPin, INPUT);

  // Inicializa a comunicação serial para debug
  Serial.begin(9600);

  // Conecta ao WiFi
  WiFi.begin("Wokwi-GUEST", "");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi!");
}

void loop() {
  // Lê o valor do LDR
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

      // Incrementa o contador e guarda o tempo
      contadorBotoes++;
      tempoUltimoPressionamento = tempoAtual;

      Serial.print("Contagem de pressionamentos: ");
      Serial.println(contadorBotoes);
    }
  }

  estadoAnteriorBotao = leituraAtualBotao;

  // Reseta o contador se o botão não for pressionado por um tempo
  if (tempoAtual - tempoUltimoPressionamento > intervaloResetContador) {
    contadorBotoes = 0;
  }

  // Se o botão foi pressionado 3 vezes e o semáforo está fechado
  if (contadorBotoes >= 3 && digitalRead(led_vermelho) == HIGH) {
    contadorBotoes = 0; // Reseta o contador depois de enviar o alerta

    // Envia o alerta via HTTP
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String enderecoServidor = "http://www.google.com.br/"; // Exemplo de URL

      http.begin(enderecoServidor.c_str());
      int codigoRespostaHTTP = http.GET();

      if (codigoRespostaHTTP > 0) {
        Serial.print("Resposta HTTP: ");
        Serial.println(codigoRespostaHTTP);
      } else {
        Serial.print("Erro na requisição HTTP: ");
        Serial.println(codigoRespostaHTTP);
      }
      http.end();
    } else {
      Serial.println("Sem WiFi. Não deu pra enviar o alerta.");
    }
  }

  botaoPressionado = false; // Reseta o estado do botão após a leitura

  // Controle do semáforo
  if (statusLDR <= limiar) {
    // Modo noturno: LED amarelo pisca
    digitalWrite(led_verde, LOW);
    digitalWrite(led_vermelho, LOW);
    digitalWrite(led_amarelo, HIGH);
    delay(500);
    digitalWrite(led_amarelo, LOW);
    delay(500);
  } else {
    // Modo normal
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
