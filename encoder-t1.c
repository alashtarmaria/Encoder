// === TANIMLAR ===
#define THRESHOLD 511 // ADC esigi (3.3V için 1.65V)

int encoder_position = 0;
char last_direction = 'X';  // Baslangiçta bilinmiyor

// === UART3 Baslat ===
void UART3_Init_Config(unsigned long baudrate) {
    PPS_Mapping(69, _INPUT, _U3RX);  // UART3 RX = RP69
    PPS_Mapping(68, _OUTPUT, _U3TX); // UART3 TX = RP68
    UART3_Init(baudrate);
    Delay_ms(100);
}

// === ADC Baslat ===
void ADC_Init() {
    AD1CON1 = 0x0000;
    AD1CON1bits.SSRC = 7;
    AD1CON2 = 0x0000;
    AD1CON3 = 0x0002;
    AD1CON1bits.ADON = 1; // ADC aktif
}

// === Belirli kanaldan ADC oku ===
unsigned int Read_ADC_Channel(unsigned char channel) {
    AD1CHS0bits.CH0SA = channel;
    AD1CON1bits.SAMP = 1;
    Delay_us(10);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

// === ENCODER OKUMA ===
// Phase A = AN17 (Channel 17)
// Phase B = AN16 (Channel 16)
// Yazilimda takas yapiyoruz
void Encoder_Read() {
    static unsigned char prev_state = 0;

    unsigned int adc_B = Read_ADC_Channel(16); // AN16 = Phase B
    unsigned int adc_A = Read_ADC_Channel(17); // AN17 = Phase A

    unsigned char A = (adc_A > THRESHOLD) ? 1 : 0;
    unsigned char B = (adc_B > THRESHOLD) ? 1 : 0;

    unsigned char current_state = (A << 1) | B;
    char new_direction = last_direction;

    if ((prev_state == 0 && current_state == 1) ||
        (prev_state == 1 && current_state == 3) ||
        (prev_state == 3 && current_state == 2) ||
        (prev_state == 2 && current_state == 0)) {
        encoder_position++;
        new_direction = 'R';
    }
    else if ((prev_state == 0 && current_state == 2) ||
             (prev_state == 2 && current_state == 3) ||
             (prev_state == 3 && current_state == 1) ||
             (prev_state == 1 && current_state == 0)) {
        encoder_position--;
        new_direction = 'L';
    }

    prev_state = current_state;

    // S yönü tamamen yok, sadece R veya L yazdirilir
    if (new_direction != last_direction) {
        last_direction = new_direction;

        if (new_direction == 'R' || new_direction == 'L') {
            char buffer[50];
            sprintf(buffer, "POS: %d, DIR: %c\r\n", encoder_position, last_direction);
            UART3_Write_Text(buffer);
        }
    }
}

// === ANA PROGRAM ===
void main() {
    PLLFBD = 70;     // Frekans ayari
    CLKDIV = 0x0000;

    UART3_Init_Config(9600);
    UART3_Write_Text("ENCODER TEST BASLADI\r\n");

    ADC_Init();

    while (1) {
        Encoder_Read();
        Delay_ms(10); // Istersen daha hizli/az yapabilirsin
    }
}
