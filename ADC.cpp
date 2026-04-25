/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : ADC.cpp
* Dependencias   : BSP.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Conversor Analógico-Digital para leer las entradas de los
*                  potenciometros del sistema
* Tareas:
*    - Inicializar el módulo ADC del ESP32
*    - Configurar los canales según los potenciómetros activos
*    - Leer valor crudo (0–4095) y valor binarizado (0 o 1)
****************************************************************************/

#include "BSP.h"

// ----------------------------------------------------------------
//  ADC_Init
//  Descripción : Configura la resolución del ADC y prepara los
//                pines de entrada analógica para los N canales.
//  Parámetros  : numChannels - número de potenciómetros (2–5)
// ----------------------------------------------------------------
void ADC_Init(int numChannels)
{
    // El ESP32 soporta resoluciones de 9 a 12 bits.
    // Usamos 12 bits para máxima precisión (0–4095).
    analogReadResolution(BSP_ADC_RESOLUTION);

    // Los pines del ESP32 en modo ADC son de solo entrada;
    // no requieren pinMode() explícito, pero lo declaramos
    // para documentar claramente cuáles se van a usar.
    for (int ch = 0; ch < numChannels; ch++) 
    {
        pinMode(g_potPins[ch], INPUT);
    }

    Serial.printf("[ADC] Inicializado: %d canal(es), resolución %d bits, max=%d\n",
                  numChannels, BSP_ADC_RESOLUTION, BSP_ADC_MAX);
}

// ----------------------------------------------------------------
//  ADC_ReadRaw
//  Descripción : Lee el valor analógico crudo de un canal.
//  Parámetros  : channel - índice del potenciómetro (0-based)
//  Retorna     : Entero 0–4095, o 0 si el canal es inválido
// ----------------------------------------------------------------
int ADC_ReadRaw(int channel)
{
    if (channel < 0 || channel >= g_numPots) 
    {
        return 0;
    }
    return analogRead(g_potPins[channel]);
}

// ----------------------------------------------------------------
//  ADC_ReadBinary
//  Descripción : Codifica la lectura analógica en 0 o 1 usando
//                el umbral definido en BSP (BSP_ADC_THRESHOLD).
//                Permite al sistema tratar el potenciómetro como
//                una entrada digital para el perceptrón.
//  Parámetros  : channel - índice del potenciómetro (0-based)
//  Retorna     : 1 si raw >= BSP_ADC_THRESHOLD, 0 en caso contrario
// ----------------------------------------------------------------
int ADC_ReadBinary(int channel)
{
    return (ADC_ReadRaw(channel) >= BSP_ADC_THRESHOLD) ? 1 : 0;
}
