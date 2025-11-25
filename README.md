# Radar Eletrônico com Classificação

## Visão Geral

O Radar Eletrônico com Classificação é um sistema embarcado completo desenvolvido em Zephyr RTOS para a plataforma emulada mps2_an385. O sistema simula um radar de tráfego inteligente capaz de detectar veículos, calcular sua velocidade, classificar o tipo de veículo (leve/pesado) e aplicar limites de velocidade específicos para cada categoria.

## Funcionalidades Principais

   - Detecção de Velocidade: Utiliza dois sensores magnéticos virtuais (GPIOs 5 e 6) para medir velocidade
   - Classificação de Veículos: Classifica automóveis baseado na contagem de eixos (2 eixos = leve, 3+ eixos = pesado)
   - Sistema de Infrações: Aplica limites diferentes para cada tipo de veículo
   - Captura Simulada de Placas: Sistema de câmera virtual com validação de formato Mercosul
   - Display Colorido: Feedback visual com cores ANSI (verde/amarelo/vermelho)
   - Estatísticas em Tempo Real: Monitoramento de veículos, infrações e falhas
   - Configuração Flexível: Todas as configurações via Kconfig

## Casos de Uso
   ```
   Veículo Leve (carro, moto) → Limite: 80 km/h
   Veículo Pesado (caminhão, ônibus) → Limite: 60 km/h

   Status:
   - VERDE: Dentro do limite
   - AMARELO: Acima de 90% do limite (alerta)
   - VERMELHO: Acima do limite (infração + captura de placa)
   ```

# Instruções de Configuração e Execução no QEMU

## Pré-requisitos
   ```
   # Instalar Zephyr SDK (Linux/Windows WSL/MacOS)
   # Verificar instalação
   west --version
   zephyr-env.sh

   # Clonar o projeto
   git clone https://github.com/ajl-orus/radar_zephyr.git
   cd radar-eletronico
   ```

## Estrutura do Projeto
   ```
      radar-eletronico/
   ├── CMakeLists.txt
   ├── prj.conf              # Configurações principais
   ├── prj_test.conf         # Configurações para testes
   ├── src/
   │   ├── main.c
   │   ├── radar.h
   │   ├── control_thread.c
   │   ├── sensor_thread.c
   │   ├── display_thread.c
   │   ├── camera_thread.c
   │   ├── speed_calculator.c
   │   ├── license_plate_validator.c
   │   └── radar_utils.c
   └── tests/
       ├── test_speed_calculator.c
       ├── test_vehicle_classifier.c
       └── test_license_validator.c
   ```
## Compilação e Execução

### 1. Compilação Básica
   ```
   # Compilar para plataforma mps2_an385
   west build -b mps2_an385

   # Executar no QEMU
   west build -t run
   ```

### 2. Compilação com Configurações Customizadas
   ```
   # Criar arquivo de configuração customizado (opcional)
   echo "
   CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH=90
   CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH=70
   CONFIG_RADAR_DEBUG_ENABLED=y
   " > radar_custom.conf

   # Compilar com configurações customizadas
   west build -b mps2_an385 -- -DOVERLAY_CONFIG=radar_custom.conf
   ```

### 3. Menu de Configuração Interativo
   ```
   # Abrir menu de configuração
   west build -t menuconfig

   # Navegar para: "Configurações do Radar Eletrônico"
   ```

### 4. Execução com Log Detalhado
   ```
   # Executar com output detalhado
   west build -t run | tee radar_output.log

   # Executar em background e monitorar
   west build -t run &
   tail -f build/zephyr/runners.log
   ```

### Saída Esperada no QEMU
   ```
   === Sistema Radar Eletronico Iniciado ===
   Distancia entre sensores: 500 mm
   Limite veiculos leves: 80 km/h
   Limite veiculos pesados: 60 km/h
   Limite de alerta: 90%
   Taxa de falha da camera: 10%

   Sensores inicializados - GPIO 5 (eixos) e GPIO 6 (velocidade)

   === RADAR ELETRONICO ===
   Veiculo: LEVE
   Velocidade: 65.2 km/h
   Limite: 80 km/h
   Status: NORMAL
   ```

# Explicação das Opções Kconfig

## Configurações Principais

### Parâmetros de Detecção
   ```
   CONFIG_RADAR_SENSOR_DISTANCE_MM=500
   ```
Descrição: Distância física entre os dois sensores em milímetros
Efeito: Afeta diretamente o cálculo de velocidade
Valores típicos: 300-1000 mm (30cm - 1m)

### Limites de Velocidade
   ```
   CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH=80
   CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH=60
   ```
Descrição: Velocidades máximas permitidas por categoria
Efeito: Define quando ocorre infração
Valores típicos: Leves: 60-100 km/h, Pesados: 50-80 km/h

### Sistema de Alerta
   ```
   CONFIG_RADAR_WARNING_THRESHOLD_PERCENT=90
   ```
Descrição: Percentual do limite que ativa alerta amarelo
Efeito: Aviso preventivo antes da infração
Exemplo: 90% de 80 km/h = 72 km/h (alerta)

### Configurações da Câmera
   ```
   CONFIG_RADAR_CAMERA_FAILURE_RATE_PERCENT=10
   ```
Descrição: Probabilidade de falha na captura da placa
Efeito: Simula problemas reais do sistema
Uso em testes: 0% (testes) até 50% (cenários adversos)

## Configurações Avançadas

### Temporização
   ```
   CONFIG_RADAR_DEBOUNCE_TIME_MS=50
   CONFIG_RADAR_DISPLAY_UPDATE_INTERVAL_MS=500
   CONFIG_RADAR_CAMERA_PROCESSING_TIME_MS=500
   ```
Descrição: Controle de tempos do sistema
Debounce: Evita leituras falsas dos sensores
Display: Atualização da interface
Câmera: Simula tempo de processamento

### Algoritmos de Classificação
   ```
   # Estratégia padrão
   CONFIG_RADAR_CLASSIFICATION_AXLE_COUNT=y

   # Estratégia alternativa (não implementada)
   CONFIG_RADAR_CLASSIFICATION_TIME_BASED=n
   ```

### Calibração
   ```
   CONFIG_RADAR_SPEED_CALIBRATION_FACTOR=100
   ```
Descrição: Ajuste fino do cálculo de velocidade (porcentagem)
Exemplo: 105 = +5% na velocidade calculada

## Configurações de Desenvolvimento

### Debug e Logging
   ```
   CONFIG_RADAR_DEBUG_ENABLED=y
   CONFIG_RADAR_LOG_LEVEL=4
   CONFIG_RADAR_SENSOR_SIMULATION=y
   ```
Debug: Mensagens detalhadas no console
Log Level: 0=Off, 1=Error, 2=Warning, 3=Info, 4=Debug
Simulação: Gera eventos automáticos para testes


# Descrição da Arquitetura

## Diagrama de Arquitetura
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   SENSORES      │    │   CONTROLE       │    │    DISPLAY      │
│                 │    │                  │    │                 │
│ GPIO 5 - Eixos  │───▶│  Classificação   │───▶│  Cores ANSI     │
│ GPIO 6 - Veloc. │    │  Cálculo Veloc.  │    │  Status Visual  │
└─────────────────┘    │  Detecção Infr.  │    └─────────────────┘
                       └─────────┬────────┘
                                 │
                       ┌─────────▼────────┐
                       │     CÂMERA       │
                       │                  │
                       │  Captura Placa   │
                       │  Validação       │
                       └──────────────────┘
```

## Threads do Sistema

### 1. Thread de Sensores (sensor_thread)

Prioridade: 6 (alta)
Responsabilidades:
   - Monitora GPIOs 5 e 6 via interrupts
   - Implementa máquina de estados para contagem de eixos
   - Mede tempo entre sensores para cálculo de velocidade
   - Aplica filtro de debouncing (50ms)

   #### Lógica de Classificação:
      ```
      // Máquina de estados simplificada
      if (pulso_GPIO5 && tempo_since_last_pulse > DEBOUNCE_TIME) {
         axle_count++;
         if (axle_count >= 3) type = VEHICLE_HEAVY;
         else if (axle_count == 2) type = VEHICLE_LIGHT;
      }
      ```

### 2. Thread de Controle (control_thread)

Prioridade: 5 (média-alta)
Responsabilidades:
   - Recebe dados dos sensores via message queue
   - Calcula velocidade final
   - Aplica limites específicos por tipo de veículo
   - Decide por infrações e aciona câmera
   - Gerencia estatísticas do sistema

   #### Cálculo de Velocidade:
      ```
      velocidade_kmh = (distancia_mm / 1000.0) / (tempo_ms / 3600000.0);
      velocidade_kmh *= fator_calibracao;
      ```

### 3. Thread de Display (display_thread)

Prioridade: 4 (média)
Responsabilidades:
   - Atualiza display virtual a cada 500ms
   - Aplica cores ANSI baseado no status
   - Mostra informações do veículo e limites
   - Interface colorida no console QEMU

   #### Sistema de Cores:
      ```
      // Códigos ANSI no console
         VERDE:    "\033[32m"   // Normal
         AMARELO:  "\033[33m"   // Alerta (90% limite)
         VERMELHO: "\033[31m"   // Infração
         AZUL:     "\033[34m"   // Informações
      ```

### 4. Thread da Câmera (camera_thread)
Prioridade: 3 (média-baixa)
Responsabilidades:
   - Aguarda trigger via ZBUS (apenas em infrações)
   - Simula processamento (500ms)
   - Gera placa Mercosul válida ou inválida
   - Publica resultado via ZBUS

   #### Validação de Placa Mercosul:
      ```
      // Formatos suportados:
      // NOVO: AAA1A23 (3 letras, 1 número, 1 letra, 2 números)
      // ANTIGO: AAA1234 (3 letras, 4 números)
      ```

## Comunicação Entre Threads

### Message Queues
   ```
   // Dados do veículo entre sensores e controle
   K_MSGQ_DEFINE(vehicle_data_queue, sizeof(vehicle_data_t), 10, 4);
   ```

### ZBUS Channels
   ```
   // Comunicação evento-driven entre controle e câmera
   ZBUS_CHAN_DEFINE(camera_trigger_chan, vehicle_data_t, ...);
   ZBUS_CHAN_DEFINE(camera_result_chan, camera_data_t, ...);
   ```

### Sincronização
   ```
   // Mutex para estatísticas compartilhadas
   struct k_mutex stats_mutex;

   // Semáforo para controle de sensores
   struct k_sem sensor_sem;   
   ```

# Instruções para Rodar os Testes

## Estrutura de Testes

   ```
   tests/
   ├── test_speed_calculator.c     # Testes de cálculo de velocidade
   ├── test_vehicle_classifier.c   # Testes de classificação
   ├── test_license_validator.c    # Testes de validação de placas
   └── CMakeLists.txt              # Configuração dos testes
   ```

## Execução de Testes

### 1. Testes Unitários (Recomendado)
   ```
   # Compilar e executar todos os testes
   west build -b native_posix -t run

   # Execução específica para testes
   ./build/zephyr/zephyr.elf
   ```

### 2. Testes com Configuração de Teste
   ```
   # Usar configuração otimizada para testes
   west build -b native_posix -- -DOVERLAY_CONFIG=prj_test.conf
   west build -t run
   ```

### 3. Testes Individuais
   ```
   # Executar apenas testes de velocidade
   west build -b native_posix -- -DOVERLAY_CONFIG=prj_test.conf
   ./build/zephyr/zephyr.elf -test_speed_calculator

   # Executar com verbose
   ./build/zephyr/zephyr.elf -v
   ```

## Casos de Teste Implementados

### Testes de Cálculo de Velocidade
   ```
   void test_speed_calculation(void) {
      // 500mm em 100ms = 18 km/h
      vehicle_data_t vehicle = {.time_between_sensors = 100};
      calculate_speed(&vehicle);
      zassert_true(vehicle.speed_kmh > 17.9 && vehicle.speed_kmh < 18.1);
   }
   ```

### Testes de Classificação
   ```
   void test_vehicle_classification(void) {
      // 2 eixos = veículo leve
      vehicle_data_t light_vehicle = {.axle_count = 2};
      zassert_equal(classify_vehicle(&light_vehicle), VEHICLE_LIGHT);
    
      // 3+ eixos = veículo pesado
      vehicle_data_t heavy_vehicle = {.axle_count = 3};
      zassert_equal(classify_vehicle(&heavy_vehicle), VEHICLE_HEAVY);
   }
   ```

### Testes de Validação de Placas
   ```
   void test_license_plate_validation(void) {
      // Placa Mercosul válida
      zassert_true(validate_license_plate("ABC1D23"));
      
      // Placa antiga válida
      zassert_true(validate_license_plate("ABC1234"));
      
      // Placa inválida
      zassert_false(validate_license_plate("AB1C234"));
   }
   ```

## Testes de Integração

### 1. Fluxo Completo Simulado
   ```
   # Executar com simulação ativa para teste manual
   west build -b mps2_an385 -- -DCONFIG_RADAR_SENSOR_SIMULATION=y
   west build -t run
   ```

### 2. Teste de Comunicação ZBUS
   ```
   // Verifica se a câmera é acionada em infrações
      void test_camera_trigger(void) {
         vehicle_data_t infraction_vehicle = {
            .speed_kmh = 85,  // Acima do limite leve (80)
            .type = VEHICLE_LIGHT
         };
    
         // Deve acionar câmera
         zassert_equal(check_speed_status(85, VEHICLE_LIGHT), SPEED_INFRACTION);
   }
   ```

## Validação Manual no QEMU

### 1. Verificar Cores do Display
   ```
   # Executar e observar as cores no terminal
   west build -t run

   # Esperado:
      # VERDE: Velocidades normais
      # AMARELO: Velocidades de alerta  
      # VERMELHO: Infrações detectadas
   ```

### 2. Testar Simulação de Sensores
   ```
   # Aumentar intervalo de simulação para observação
   echo "CONFIG_RADAR_SIMULATION_INTERVAL_MS=5000" >> prj_test.conf
   west build -b mps2_an385 -- -DOVERLAY_CONFIG=prj_test.conf
   ```

### 3. Monitorar Estatísticas
   ```
   # Verificar contadores no final da execução
   [STATS] Total veiculos: 15, Leves: 10, Pesados: 5, Infracoes: 2
   ```
