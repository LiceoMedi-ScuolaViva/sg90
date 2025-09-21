/*
  Servo angle from Serial (degrees)

  - Apri il Serial Monitor (115200 baud), digita un angolo tra 0 e 180 e premi Invio.
  - Sono accettati CR, LF o CR+LF come terminatori.
  - Input fuori range viene clampato a [0..180].
*/

#include <Arduino.h>
#include <Servo.h>

#define SERVO_PIN          9
#define SERIAL_BAUD        115200L
#define MIN_ANGLE_DEG      0U
#define MAX_ANGLE_DEG      180U

/* Facoltativo: specifica l'escursione del tuo servo in µs (qui classico 1000–2000). */
#define SERVO_MIN_US       500U
#define SERVO_MAX_US       2500U

static Servo g_servo;

/* Semplice linea buffer per l'input da Serial. */
#define INBUF_SIZE         16U
static char g_inbuf[INBUF_SIZE];
static uint8_t g_inlen = 0U;

static void print_help_(void)
{
  Serial.println(F("== Servo controller =="));
  Serial.println(F("Inserisci un angolo in gradi (0..180) e premi Invio."));
  Serial.println(F("Esempi: 0, 45, 90, 135, 180"));
  Serial.println(F("Scrivi '?' per ristampare questo messaggio."));
}

static void handle_line_(const char* line)
{
  /* Gestisci comando di help. */
  if ((line[0] == '?') && (line[1] == '\0')) {
    print_help_();
    return;
  }

  /* Converte in long in modo sicuro. */
  char* endptr = NULL;
  long value = strtol(line, &endptr, 10);

  /* Verifica che tutta la stringa fosse un numero (permette spazi finali). */
  while ((endptr != NULL) && (*endptr == ' ')) {
    ++endptr;
  }

  if ((endptr == NULL) || (*endptr != '\0')) {
    Serial.println(F("Input non valido. Inserisci un intero tra 0 e 180."));
    return;
  }

  /* Clamp all'intervallo consentito. */
  if (value < MIN_ANGLE_DEG) {
    value = MIN_ANGLE_DEG;
  } else if (value > MAX_ANGLE_DEG) {
    value = MAX_ANGLE_DEG;
  }

  /* Muove il servo. */
  g_servo.write(static_cast<int>(value));
  Serial.print(F("Angolo impostato: "));
  Serial.print(value);
  Serial.println(F(" deg"));
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  g_servo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);

  g_servo.write(90);

  while (!Serial) {}

  print_help_();
  Serial.print(F("Pin servo: D"));
  Serial.println(SERVO_PIN);
}

void loop()
{
  /* Leggi caratteri se presenti. */
  while (Serial.available() > 0) {
    int ch = Serial.read();
    if (ch < 0) {
      break;
    }

    /* Gestisci terminatori di riga CR/LF. */
    if ((ch == '\n') || (ch == '\r')) {
      if (g_inlen > 0U) {
        g_inbuf[g_inlen] = '\0';
        handle_line_(g_inbuf);
        g_inlen = 0U;
        g_inbuf[0] = '\0';
      }
      /* Ignora terminatori consecutivi. */
      continue;
    }

    /* Accumula caratteri numerici e spazi, evita overflow. */
    if (g_inlen < (INBUF_SIZE - 1U)) {
      g_inbuf[g_inlen] = static_cast<char>(ch);
      g_inlen++;
      g_inbuf[g_inlen] = '\0';
    } else {
      /* Buffer pieno: azzera e segnala errore. */
      g_inlen = 0U;
      g_inbuf[0] = '\0';
      Serial.println(F("Linea troppo lunga. Riprova con meno caratteri."));
      /* Svuota fino al prossimo terminatore. */
      while (Serial.available() > 0) {
        int c2 = Serial.read();
        if ((c2 == '\n') || (c2 == '\r')) {
          break;
        }
      }
    }
  }
}