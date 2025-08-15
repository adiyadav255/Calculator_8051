#include <at89x52.h>
#define lcd_data_port P2
#define keypad_port P1

__sbit __at (0xB0) rs;  // P3.0
__sbit __at (0xB1) rw;  // P3.1
__sbit __at (0xB2) en;  // P3.2

unsigned char keypad[4][4] = {
	{'7', '8', '9', '/'},   // Row 0
  {'4', '5', '6', '*'},   // Row 1
  {'1', '2', '3', '-'},   // Row 2
  {'C', '0', '=', '+'}    // Row 3, because the keypad is internally inverted
};
void delay(unsigned int i);
void LCD_command(unsigned char cmd);
void LCD_init(void);
void LCD_char(unsigned char char_data);
void LCD_String(unsigned char *str);
void LCD_int(unsigned int num);
unsigned char Read_Keypad(void);
int char_to_int(unsigned char key);
void delay(unsigned int t) {
    unsigned int i, j;
    for(i = 0; i < t; i++)
        for(j = 0; j < 127; j++);
}
unsigned char Read_Keypad(void) {
    unsigned char row, col;
    while (1) {
        for (row = 0; row < 4; row++) {
            keypad_port = 0xFF;                // Set all high
            keypad_port &= ~(1 << row);        // Drive one row LOW
            delay(2);                          

            for (col = 0; col < 4; col++) {
                if (!(keypad_port & (1 << (col + 4)))) { // Check column
                    while (!(keypad_port & (1 << (col + 4)))); // Wait release
                    return keypad[row][col];
                }
            }
        }
    }
}
void LCD_command(unsigned char cmd) {
    lcd_data_port = cmd;
    rs = 0;
    rw = 0;
    en = 1;
    delay(2);
    en = 0;
    delay(2);
}
void LCD_char(unsigned char char_data) {
    lcd_data_port = char_data;
    rs = 1;
    rw = 0;
    en = 1;
    delay(2);
    en = 0;
    delay(2);
}
void LCD_String(unsigned char *str) {
    while(*str) {
        LCD_char(*str++);
    }
}

void LCD_int(unsigned int num) {
    char str[6];//buffer to store integer as string//
    int i = 0;
    if(num == 0) {
        LCD_char('0');
        return;
    }
    while(num > 0) {
        str[i++] = (num % 10) + '0';//last digit to first digit//
        num /= 10;
    }
    while(i > 0) {
        LCD_char(str[--i]);//reverse the string for actual number//
    }
}

void LCD_init(void) {
    delay(20);
    LCD_command(0x38);
    LCD_command(0x0C);
    LCD_command(0x06);
    LCD_command(0x01);
    LCD_command(0x80);
}

int char_to_int(unsigned char key) {
    if (key >= '0' && key <= '9')
        return key - '0';
    return -1;
}

void main(void) {
    LCD_init();
    LCD_String("SelectOp:+ - * /");

    unsigned char key;
    char op = 0;
    int n1 = -1, n2 = -1,result = 0;

    while (1) {
        key = Read_Keypad();

        if (key == 'C') {
            LCD_command(0x01);
            LCD_String("Select Op: + - * /");
            op = 0; n1 = -1; n2 = -1; result = 0;
            continue;
        }

        if ((key == '+') || (key == '-') || (key == '*') || (key == '/')) {
            op = key;
            LCD_command(0x01);
            LCD_char(op);
            continue;
        }

        if (char_to_int(key) != -1) {
            if (n1 == -1) {
                n1 = char_to_int(key);
                LCD_char(' '); LCD_int(n1);
            } else if (n2 == -1) {
                n2 = char_to_int(key);
                LCD_char(' '); LCD_int(n2);
            }
            continue;
        }

        if (key == '=' && n1 != -1 && n2 != -1 && op != 0) {
            LCD_command(0x01);
            switch (op) {
                case '+': result = n1 + n2; break;
                case '-': result = n1 - n2; break;
                case '*': result = n1 * n2; break;
                case '/':
                    if (n2 != 0) {
                        LCD_String("Quotient:");
                        LCD_command(0xC0);
                        LCD_int(n1 / n2);
                        delay(5000);
                        LCD_command(0x01);
                        LCD_String("Remainder:");
                        LCD_command(0xC0);
                        LCD_int(n1 % n2);
                        delay(5000);
                        break;
                    } else {
                        LCD_String("Divide by 0 Err");
                        delay(5000);
                        break;
                    }
                default: break;
            }

            if (op != '/') {
                LCD_String("Result:");
                LCD_command(0xC0);
                LCD_int(result);
                delay(5000);
            }

            op = 0; n1 = -1; n2 = -1;
            LCD_command(0x01);
            LCD_String("Select Op: + - * /");
        }
    }
}
