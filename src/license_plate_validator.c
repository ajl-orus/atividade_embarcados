#include "radar.h"

bool validate_license_plate(const char *plate)
{
    // Verifica formato Mercosul: AAA1A23 ou AAA1234 (formato antigo)
    if (strlen(plate) != 7) {
        return false;
    }
    
    // Verifica letras nas posições 1-3
    for (int i = 0; i < 3; i++) {
        if (!isalpha(plate[i])) {
            return false;
        }
    }
    
    // Verifica dígitos e letra no formato novo
    if (isalpha(plate[3]) && isdigit(plate[4]) && isalpha(plate[5])) {
        // Formato novo: AAA1A23
        if (!isdigit(plate[6])) return false;
    } else if (isdigit(plate[3]) && isdigit(plate[4]) && isdigit(plate[5])) {
        // Formato antigo: AAA1234
        if (!isdigit(plate[6])) return false;
    } else {
        return false;
    }
    
    return true;
}

void simulate_license_plate(char *plate, bool *valid)
{
    // Gera placa aleatória
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char digits[] = "0123456789";
    
    // Formato Mercosul (70% das vezes) ou antigo (30%)
    if (k_uptime_get_32() % 10 < 7) {
        // Formato Mercosul: AAA1A23
        for (int i = 0; i < 3; i++) {
            plate[i] = letters[k_uptime_get_32() % 26];
        }
        plate[3] = digits[k_uptime_get_32() % 10];
        plate[4] = letters[k_uptime_get_32() % 26];
        plate[5] = digits[k_uptime_get_32() % 10];
        plate[6] = digits[k_uptime_get_32() % 10];
    } else {
        // Formato antigo: AAA1234
        for (int i = 0; i < 3; i++) {
            plate[i] = letters[k_uptime_get_32() % 26];
        }
        for (int i = 3; i < 7; i++) {
            plate[i] = digits[k_uptime_get_32() % 10];
        }
    }
    plate[7] = '\0';
    
    // Simula falha baseada na taxa configurada
    *valid = ((k_uptime_get_32() % 100) >= CAMERA_FAILURE_RATE);
    
    // Se for inválida, corrompe a placa
    if (!*valid) {
        plate[2] = '0'; // Caractere inválido na posição de letra
    }
}