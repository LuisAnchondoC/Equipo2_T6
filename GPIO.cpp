/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : GPIO.cpp
* Dependencias   : BSP.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Componente HAL, Entradas y Salidas de Propósito General.
*                  - Inicializa el módulo GPIO (LEDs y botón)
*                  - Escritura de bit en un pin de salida
*                  - Lectura de bit desde un pin de entrada
****************************************************************************/
#include "BSP.h"

// ----------------------------------------------------------------
//  GPIO_Init
//  Descripción : Inicializa todos los pines GPIO del sistema:
//                - LED de estado      - salida, apagado al inicio
//                - LED de salida ML   - salida, apagado al inicio
//                - Botón              - entrada con pull-up interno
// ----------------------------------------------------------------
void GPIO_Init(void)
{
    // Configurar LEDs como salidas y garantizar estado inicial apagado
    pinMode(BSP_PIN_LED_STATE,  OUTPUT);
    pinMode(BSP_PIN_LED_OUTPUT, OUTPUT);
    digitalWrite(BSP_PIN_LED_STATE,  LOW);
    digitalWrite(BSP_PIN_LED_OUTPUT, LOW);

    // Botón con resistencia pull-up interna.
    // Lógica invertida: presionado = LOW, suelto = HIGH.
    pinMode(BSP_PIN_BUTTON, INPUT_PULLUP);

    Serial.printf("[GPIO] Módulo inicializado\n");
    Serial.printf("[GPIO]  LED estado  - pin %d (salida)\n", BSP_PIN_LED_STATE);
    Serial.printf("[GPIO]  LED salida  - pin %d (salida)\n", BSP_PIN_LED_OUTPUT);
    Serial.printf("[GPIO]  Botón       - pin %d (entrada pull-up)\n", BSP_PIN_BUTTON);
}

// ----------------------------------------------------------------
//  GPIO_WritePin
//  Descripción : Escribe un valor lógico en un pin de salida.
//  Parámetros  : pin   - número de GPIO a escribir
//                value - true = HIGH (3.3 V), false = LOW (0 V)
// ----------------------------------------------------------------
void GPIO_WritePin(int pin, bool value)
{
    digitalWrite(pin, value ? HIGH : LOW);
}

// ----------------------------------------------------------------
//  GPIO_ReadPin
//  Descripción : Lee el estado lógico de un pin de entrada.
//                Considera la lógica invertida del botón
//                (pull-up interno: presionado → LOW → retorna true).
//  Parámetros  : pin - número de GPIO a leer
//  Retorna     : true si el botón está presionado (pin en LOW)
// ----------------------------------------------------------------
bool GPIO_ReadPin(int pin)
{
    // El botón usa pull-up: LOW significa presionado.
    // Invertimos para devolver true = "botón activo".
    return (digitalRead(pin) == LOW);
}
