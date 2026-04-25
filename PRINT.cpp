/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : PRINT.cpp
* Dependencias   : BSP.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Componente HAL: Consola de depuración Serial
****************************************************************************/

#include "BSP.h"

// ----------------------------------------------------------------
//  PRINT_Init
//  Descripción : Inicializa el puerto serial con la velocidad
//                definida en BSP e imprime el banner de arranque.
//  Parámetros  : baudRate - velocidad en baudios (ej. 115200)
// ----------------------------------------------------------------
void PRINT_Init(int baudRate)
{
    Serial.begin(baudRate);
    delay(500);         // Esperar a que el terminal se conecte
    PRINT_Banner();
}

// ----------------------------------------------------------------
//  PRINT_Banner
//  Descripción : Imprime el encabezado del sistema al arranque.
// ----------------------------------------------------------------
void PRINT_Banner(void)
{
    Serial.println("\n================================================");
    Serial.println("  Perceptron Simple con LMS - ESP32 + FreeRTOS");
    Serial.println("  Laboratorio de Machine Learning  |  ITCH");
    Serial.println("  Tarea 6: Perceptrón embebido");
    Serial.println("================================================\n");
}

// ----------------------------------------------------------------
//  PRINT_Separator
//  Descripción : Imprime una línea divisoria para organizar la
//                salida en el monitor serial.
// ----------------------------------------------------------------
void PRINT_Separator(void)
{
    Serial.println("------------------------------------------------");
}

// ----------------------------------------------------------------
//  PRINT_ADCValues
//  Descripción : Imprime en una sola línea el valor crudo y el
//                valor binarizado de cada canal ADC activo.
//                Llamada periódicamente por la tarea ADCPrint.
//  Parámetros  : numChannels - cantidad de potenciómetros activos
// ----------------------------------------------------------------
void PRINT_ADCValues(int numChannels)
{
    // Valores crudos
    Serial.print("[ADC] Raw:  ");
    for (int i = 0; i < numChannels; i++) 
    {
        Serial.printf("P%d=%4d  ", i + 1, ADC_ReadRaw(i));
    }

    // Valores binarizados (umbral definido en BSP)
    Serial.print("| Binario: [");
    for (int i = 0; i < numChannels; i++) 
    {
        Serial.print(ADC_ReadBinary(i));
        if (i < numChannels - 1) Serial.print(", ");
    }
    Serial.println("]");
}
