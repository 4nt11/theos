CS111 Harvard - Printing: https://www.read.seas.harvard.edu/~kohler/class/05f-osp/notes/lec03.html
OSDev - VGA Hardware: https://wiki.osdev.org/VGA_Hardware

# Important VGA formulas
## Left shifting and ORing for hex values.

Instead of using hexadecimal values to reffer to ASCII characters, we can instead do:
`(color << 8) | char`
Where `color` is a decimal value and `char` is the character itself. For example:
Using `color` 3 and `char` B, the formula then becomes:
`(3 << 8) | B`, where
768d | B (0x42, or 00101010b)
0011 0000 0000 | 0010 1010, so

0011 0000 0000
     0100 0010
--------------
0011 0100 0010, which is 0x342 in little endian, or 0x423 in big endian, where 0x42 is the character 'B' and 0x03 is the color '3'.

## VGA Index calculation
In some cases, we'll use the following formula:
(y * VGA_WIDTH) + x, where `y` is a position in a VGA row and `x` is a position in the VGA column. For example:
(0 * 80) + 0 = 0, so the first character;
(1 * 80) + 1 = 81, so the 81st character; but what? why? wouldn't that be the second character? **NO!** because `y` is the **ROW**, not the **COLUMN!** so, the second character would be:
(0 * 80) + 1 = 1, this is the second character!

# Memory addresses

- `0xB8000`: memory address for color displays to display characters in the screen.
- `0xB0000`: memory address for monochrome displays to display characters in the screen.

# Color Table

| Color number | Color name   | RGB value   | Hex value |
| ------------ | ------------ | ----------- | --------- |
| 0            | Black        | 0 0 0       | 00 00 00  |
| 1            | Blue         | 0 0 170     | 00 00 AA  |
| 2            | Green        | 0 170 0     | 00 AA 00  |
| 3            | Cyan         | 0 170 170   | 00 AA AA  |
| 4            | Red          | 170 0 0     | AA 00 00  |
| 5            | Purple       | 170 0 170   | AA 00 AA  |
| 6            | Brown        | 170 85 0    | AA 55 00  |
| 7            | Gray         | 170 170 170 | AA AA AA  |
| 8            | Dark Gray    | 85 85 85    | 55 55 55  |
| 9            | Light Blue   | 85 85 255   | 55 55 FF  |
| 10           | Light Green  | 85 255 85   | 55 FF 55  |
| 11           | Light Cyan   | 85 255 255  | 55 FF FF  |
| 12           | Light Red    | 255 85 85   | FF 55 55  |
| 13           | Light Purple | 255 85 255  | FF 55 FF  |
| 14           | Yellow       | 255 255 85  | FF FF 55  |
| 15           | White        | 255 255 255 | FF FF FF  |
# ASCII Tables

## ASCII Control Characters

| DEC                                                 | OCT | HEX | BIN      | Symbol | HTML Number | HTML Name | Description                 |
| --------------------------------------------------- | --- | --- | -------- | ------ | ----------- | --------- | --------------------------- |
| [0](https://www.ascii-code.com/0 "ASCII Code 0")    | 000 | 00  | 00000000 | NUL    | &#00;       |           | Null character              |
| [1](https://www.ascii-code.com/1 "ASCII Code 1")    | 001 | 01  | 00000001 | SOH    | &#01;       |           | Start of Heading            |
| [2](https://www.ascii-code.com/2 "ASCII Code 2")    | 002 | 02  | 00000010 | STX    | &#02;       |           | Start of Text               |
| [3](https://www.ascii-code.com/3 "ASCII Code 3")    | 003 | 03  | 00000011 | ETX    | &#03;       |           | End of Text                 |
| [4](https://www.ascii-code.com/4 "ASCII Code 4")    | 004 | 04  | 00000100 | EOT    | &#04;       |           | End of Transmission         |
| [5](https://www.ascii-code.com/5 "ASCII Code 5")    | 005 | 05  | 00000101 | ENQ    | &#05;       |           | Enquiry                     |
| [6](https://www.ascii-code.com/6 "ASCII Code 6")    | 006 | 06  | 00000110 | ACK    | &#06;       |           | Acknowledge                 |
| [7](https://www.ascii-code.com/7 "ASCII Code 7")    | 007 | 07  | 00000111 | BEL    | &#07;       |           | Bell, Alert                 |
| [8](https://www.ascii-code.com/8 "ASCII Code 8")    | 010 | 08  | 00001000 | BS     | &#08;       |           | Backspace                   |
| [9](https://www.ascii-code.com/9 "ASCII Code 9")    | 011 | 09  | 00001001 | HT     | &#09;       |           | Horizontal Tab              |
| [10](https://www.ascii-code.com/10 "ASCII Code 10") | 012 | 0A  | 00001010 | LF     | &#10;       |           | Line Feed                   |
| [11](https://www.ascii-code.com/11 "ASCII Code 11") | 013 | 0B  | 00001011 | VT     | &#11;       |           | Vertical Tabulation         |
| [12](https://www.ascii-code.com/12 "ASCII Code 12") | 014 | 0C  | 00001100 | FF     | &#12;       |           | Form Feed                   |
| [13](https://www.ascii-code.com/13 "ASCII Code 13") | 015 | 0D  | 00001101 | CR     | &#13;       |           | Carriage Return             |
| [14](https://www.ascii-code.com/14 "ASCII Code 14") | 016 | 0E  | 00001110 | SO     | &#14;       |           | Shift Out                   |
| [15](https://www.ascii-code.com/15 "ASCII Code 15") | 017 | 0F  | 00001111 | SI     | &#15;       |           | Shift In                    |
| [16](https://www.ascii-code.com/16 "ASCII Code 16") | 020 | 10  | 00010000 | DLE    | &#16;       |           | Data Link Escape            |
| [17](https://www.ascii-code.com/17 "ASCII Code 17") | 021 | 11  | 00010001 | DC1    | &#17;       |           | Device Control One (XON)    |
| [18](https://www.ascii-code.com/18 "ASCII Code 18") | 022 | 12  | 00010010 | DC2    | &#18;       |           | Device Control Two          |
| [19](https://www.ascii-code.com/19 "ASCII Code 19") | 023 | 13  | 00010011 | DC3    | &#19;       |           | Device Control Three (XOFF) |
| [20](https://www.ascii-code.com/20 "ASCII Code 20") | 024 | 14  | 00010100 | DC4    | &#20;       |           | Device Control Four         |
| [21](https://www.ascii-code.com/21 "ASCII Code 21") | 025 | 15  | 00010101 | NAK    | &#21;       |           | Negative Acknowledge        |
| [22](https://www.ascii-code.com/22 "ASCII Code 22") | 026 | 16  | 00010110 | SYN    | &#22;       |           | Synchronous Idle            |
| [23](https://www.ascii-code.com/23 "ASCII Code 23") | 027 | 17  | 00010111 | ETB    | &#23;       |           | End of Transmission Block   |
| [24](https://www.ascii-code.com/24 "ASCII Code 24") | 030 | 18  | 00011000 | CAN    | &#24;       |           | Cancel                      |
| [25](https://www.ascii-code.com/25 "ASCII Code 25") | 031 | 19  | 00011001 | EM     | &#25;       |           | End of medium               |
| [26](https://www.ascii-code.com/26 "ASCII Code 26") | 032 | 1A  | 00011010 | SUB    | &#26;       |           | Substitute                  |
| [27](https://www.ascii-code.com/27 "ASCII Code 27") | 033 | 1B  | 00011011 | ESC    | &#27;       |           | Escape                      |
| [28](https://www.ascii-code.com/28 "ASCII Code 28") | 034 | 1C  | 00011100 | FS     | &#28;       |           | File Separator              |
| [29](https://www.ascii-code.com/29 "ASCII Code 29") | 035 | 1D  | 00011101 | GS     | &#29;       |           | Group Separator             |
| [30](https://www.ascii-code.com/30 "ASCII Code 30") | 036 | 1E  | 00011110 | RS     | &#30;       |           | Record Separator            |
| [31](https://www.ascii-code.com/31 "ASCII Code 31") | 037 | 1F  | 00011111 | US     | &#31;       |           | Unit Separator              |
## ASCII Printable Characters

| DEC                                                    | OCT | HEX | BIN      | Symbol  | HTML Number | HTML Name | Description                            |
| ------------------------------------------------------ | --- | --- | -------- | ------- | ----------- | --------- | :------------------------------------- |
| [32](https://www.ascii-code.com/32 "ASCII Code 32")    | 040 | 20  | 00100000 | SP      | &#32;       |           | Space                                  |
| [33](https://www.ascii-code.com/33 "ASCII Code 33")    | 041 | 21  | 00100001 | !       | &#33;       | &excl;    | Exclamation mark                       |
| [34](https://www.ascii-code.com/34 "ASCII Code 34")    | 042 | 22  | 00100010 | "       | &#34;       | &quot;    | Double quotes (or speech marks)        |
| [35](https://www.ascii-code.com/35 "ASCII Code 35")    | 043 | 23  | 00100011 | #       | &#35;       | &num;     | Number sign                            |
| [36](https://www.ascii-code.com/36 "ASCII Code 36")    | 044 | 24  | 00100100 | $       | &#36;       | &dollar;  | Dollar                                 |
| [37](https://www.ascii-code.com/37 "ASCII Code 37")    | 045 | 25  | 00100101 | %       | &#37;       | &percnt;  | Per cent sign                          |
| [38](https://www.ascii-code.com/38 "ASCII Code 38")    | 046 | 26  | 00100110 | &       | &#38;       | &amp;     | Ampersand                              |
| [39](https://www.ascii-code.com/39 "ASCII Code 39")    | 047 | 27  | 00100111 | '       | &#39;       | &apos;    | Single quote                           |
| [40](https://www.ascii-code.com/40 "ASCII Code 40")    | 050 | 28  | 00101000 | (       | &#40;       | &lparen;  | Open parenthesis (or open bracket)     |
| [41](https://www.ascii-code.com/41 "ASCII Code 41")    | 051 | 29  | 00101001 | )       | &#41;       | &rparen;  | Close parenthesis (or close bracket)   |
| [42](https://www.ascii-code.com/42 "ASCII Code 42")    | 052 | 2A  | 00101010 | *       | &#42;       | &ast;     | Asterisk                               |
| [43](https://www.ascii-code.com/43 "ASCII Code 43")    | 053 | 2B  | 00101011 | +       | &#43;       | &plus;    | Plus                                   |
| [44](https://www.ascii-code.com/44 "ASCII Code 44")    | 054 | 2C  | 00101100 | ,       | &#44;       | &comma;   | Comma                                  |
| [45](https://www.ascii-code.com/45 "ASCII Code 45")    | 055 | 2D  | 00101101 | -       | &#45;       |           | Hyphen-minus                           |
| [46](https://www.ascii-code.com/46 "ASCII Code 46")    | 056 | 2E  | 00101110 | .       | &#46;       | &period;  | Period, dot or full stop               |
| [47](https://www.ascii-code.com/47 "ASCII Code 47")    | 057 | 2F  | 00101111 | /       | &#47;       | &sol;     | Slash or divide                        |
| [48](https://www.ascii-code.com/48 "ASCII Code 48")    | 060 | 30  | 00110000 | 0       | &#48;       |           | Zero                                   |
| [49](https://www.ascii-code.com/49 "ASCII Code 49")    | 061 | 31  | 00110001 | 1       | &#49;       |           | One                                    |
| [50](https://www.ascii-code.com/50 "ASCII Code 50")    | 062 | 32  | 00110010 | 2       | &#50;       |           | Two                                    |
| [51](https://www.ascii-code.com/51 "ASCII Code 51")    | 063 | 33  | 00110011 | 3       | &#51;       |           | Three                                  |
| [52](https://www.ascii-code.com/52 "ASCII Code 52")    | 064 | 34  | 00110100 | 4       | &#52;       |           | Four                                   |
| [53](https://www.ascii-code.com/53 "ASCII Code 53")    | 065 | 35  | 00110101 | 5       | &#53;       |           | Five                                   |
| [54](https://www.ascii-code.com/54 "ASCII Code 54")    | 066 | 36  | 00110110 | 6       | &#54;       |           | Six                                    |
| [55](https://www.ascii-code.com/55 "ASCII Code 55")    | 067 | 37  | 00110111 | 7       | &#55;       |           | Seven                                  |
| [56](https://www.ascii-code.com/56 "ASCII Code 56")    | 070 | 38  | 00111000 | 8       | &#56;       |           | Eight                                  |
| [57](https://www.ascii-code.com/57 "ASCII Code 57")    | 071 | 39  | 00111001 | 9       | &#57;       |           | Nine                                   |
| [58](https://www.ascii-code.com/58 "ASCII Code 58")    | 072 | 3A  | 00111010 | :       | &#58;       | &colon;   | Colon                                  |
| [59](https://www.ascii-code.com/59 "ASCII Code 59")    | 073 | 3B  | 00111011 | ;       | &#59;       | &semi;    | Semicolon                              |
| [60](https://www.ascii-code.com/60 "ASCII Code 60")    | 074 | 3C  | 00111100 | <       | &#60;       | &lt;      | Less than (or open angled bracket)     |
| [61](https://www.ascii-code.com/61 "ASCII Code 61")    | 075 | 3D  | 00111101 | =       | &#61;       | &equals;  | Equals                                 |
| [62](https://www.ascii-code.com/62 "ASCII Code 62")    | 076 | 3E  | 00111110 | >       | &#62;       | &gt;      | Greater than (or close angled bracket) |
| [63](https://www.ascii-code.com/63 "ASCII Code 63")    | 077 | 3F  | 00111111 | ?       | &#63;       | &quest;   | Question mark                          |
| [64](https://www.ascii-code.com/64 "ASCII Code 64")    | 100 | 40  | 01000000 | @       | &#64;       | &commat;  | At sign                                |
| [65](https://www.ascii-code.com/65 "ASCII Code 65")    | 101 | 41  | 01000001 | A       | &#65;       |           | Uppercase A                            |
| [66](https://www.ascii-code.com/66 "ASCII Code 66")    | 102 | 42  | 01000010 | B       | &#66;       |           | Uppercase B                            |
| [67](https://www.ascii-code.com/67 "ASCII Code 67")    | 103 | 43  | 01000011 | C       | &#67;       |           | Uppercase C                            |
| [68](https://www.ascii-code.com/68 "ASCII Code 68")    | 104 | 44  | 01000100 | D       | &#68;       |           | Uppercase D                            |
| [69](https://www.ascii-code.com/69 "ASCII Code 69")    | 105 | 45  | 01000101 | E       | &#69;       |           | Uppercase E                            |
| [70](https://www.ascii-code.com/70 "ASCII Code 70")    | 106 | 46  | 01000110 | F       | &#70;       |           | Uppercase F                            |
| [71](https://www.ascii-code.com/71 "ASCII Code 71")    | 107 | 47  | 01000111 | G       | &#71;       |           | Uppercase G                            |
| [72](https://www.ascii-code.com/72 "ASCII Code 72")    | 110 | 48  | 01001000 | H       | &#72;       |           | Uppercase H                            |
| [73](https://www.ascii-code.com/73 "ASCII Code 73")    | 111 | 49  | 01001001 | I       | &#73;       |           | Uppercase I                            |
| [74](https://www.ascii-code.com/74 "ASCII Code 74")    | 112 | 4A  | 01001010 | J       | &#74;       |           | Uppercase J                            |
| [75](https://www.ascii-code.com/75 "ASCII Code 75")    | 113 | 4B  | 01001011 | K       | &#75;       |           | Uppercase K                            |
| [76](https://www.ascii-code.com/76 "ASCII Code 76")    | 114 | 4C  | 01001100 | L       | &#76;       |           | Uppercase L                            |
| [77](https://www.ascii-code.com/77 "ASCII Code 77")    | 115 | 4D  | 01001101 | M       | &#77;       |           | Uppercase M                            |
| [78](https://www.ascii-code.com/78 "ASCII Code 78")    | 116 | 4E  | 01001110 | N       | &#78;       |           | Uppercase N                            |
| [79](https://www.ascii-code.com/79 "ASCII Code 79")    | 117 | 4F  | 01001111 | O       | &#79;       |           | Uppercase O                            |
| [80](https://www.ascii-code.com/80 "ASCII Code 80")    | 120 | 50  | 01010000 | P       | &#80;       |           | Uppercase P                            |
| [81](https://www.ascii-code.com/81 "ASCII Code 81")    | 121 | 51  | 01010001 | Q       | &#81;       |           | Uppercase Q                            |
| [82](https://www.ascii-code.com/82 "ASCII Code 82")    | 122 | 52  | 01010010 | R       | &#82;       |           | Uppercase R                            |
| [83](https://www.ascii-code.com/83 "ASCII Code 83")    | 123 | 53  | 01010011 | S       | &#83;       |           | Uppercase S                            |
| [84](https://www.ascii-code.com/84 "ASCII Code 84")    | 124 | 54  | 01010100 | T       | &#84;       |           | Uppercase T                            |
| [85](https://www.ascii-code.com/85 "ASCII Code 85")    | 125 | 55  | 01010101 | U       | &#85;       |           | Uppercase U                            |
| [86](https://www.ascii-code.com/86 "ASCII Code 86")    | 126 | 56  | 01010110 | V       | &#86;       |           | Uppercase V                            |
| [87](https://www.ascii-code.com/87 "ASCII Code 87")    | 127 | 57  | 01010111 | W       | &#87;       |           | Uppercase W                            |
| [88](https://www.ascii-code.com/88 "ASCII Code 88")    | 130 | 58  | 01011000 | X       | &#88;       |           | Uppercase X                            |
| [89](https://www.ascii-code.com/89 "ASCII Code 89")    | 131 | 59  | 01011001 | Y       | &#89;       |           | Uppercase Y                            |
| [90](https://www.ascii-code.com/90 "ASCII Code 90")    | 132 | 5A  | 01011010 | Z       | &#90;       |           | Uppercase Z                            |
| [91](https://www.ascii-code.com/91 "ASCII Code 91")    | 133 | 5B  | 01011011 | [       | &#91;       | &lsqb;    | Opening bracket                        |
| [92](https://www.ascii-code.com/92 "ASCII Code 92")    | 134 | 5C  | 01011100 | \|&#92; | &bsol;      | Backslash |                                        |
| [93](https://www.ascii-code.com/93 "ASCII Code 93")    | 135 | 5D  | 01011101 | ]       | &#93;       | &rsqb;    | Closing bracket                        |
| [94](https://www.ascii-code.com/94 "ASCII Code 94")    | 136 | 5E  | 01011110 | ^       | &#94;       | &Hat;     | Caret - circumflex                     |
| [95](https://www.ascii-code.com/95 "ASCII Code 95")    | 137 | 5F  | 01011111 | _       | &#95;       | &lowbar;  | Underscore                             |
| [96](https://www.ascii-code.com/96 "ASCII Code 96")    | 140 | 60  | 01100000 | `       | &#96;       | &grave;   | Grave accent                           |
| [97](https://www.ascii-code.com/97 "ASCII Code 97")    | 141 | 61  | 01100001 | a       | &#97;       |           | Lowercase a                            |
| [98](https://www.ascii-code.com/98 "ASCII Code 98")    | 142 | 62  | 01100010 | b       | &#98;       |           | Lowercase b                            |
| [99](https://www.ascii-code.com/99 "ASCII Code 99")    | 143 | 63  | 01100011 | c       | &#99;       |           | Lowercase c                            |
| [100](https://www.ascii-code.com/100 "ASCII Code 100") | 144 | 64  | 01100100 | d       | &#100;      |           | Lowercase d                            |
| [101](https://www.ascii-code.com/101 "ASCII Code 101") | 145 | 65  | 01100101 | e       | &#101;      |           | Lowercase e                            |
| [102](https://www.ascii-code.com/102 "ASCII Code 102") | 146 | 66  | 01100110 | f       | &#102;      |           | Lowercase f                            |
| [103](https://www.ascii-code.com/103 "ASCII Code 103") | 147 | 67  | 01100111 | g       | &#103;      |           | Lowercase g                            |
| [104](https://www.ascii-code.com/104 "ASCII Code 104") | 150 | 68  | 01101000 | h       | &#104;      |           | Lowercase h                            |
| [105](https://www.ascii-code.com/105 "ASCII Code 105") | 151 | 69  | 01101001 | i       | &#105;      |           | Lowercase i                            |
| [106](https://www.ascii-code.com/106 "ASCII Code 106") | 152 | 6A  | 01101010 | j       | &#106;      |           | Lowercase j                            |
| [107](https://www.ascii-code.com/107 "ASCII Code 107") | 153 | 6B  | 01101011 | k       | &#107;      |           | Lowercase k                            |
| [108](https://www.ascii-code.com/108 "ASCII Code 108") | 154 | 6C  | 01101100 | l       | &#108;      |           | Lowercase l                            |
| [109](https://www.ascii-code.com/109 "ASCII Code 109") | 155 | 6D  | 01101101 | m       | &#109;      |           | Lowercase m                            |
| [110](https://www.ascii-code.com/110 "ASCII Code 110") | 156 | 6E  | 01101110 | n       | &#110;      |           | Lowercase n                            |
| [111](https://www.ascii-code.com/111 "ASCII Code 111") | 157 | 6F  | 01101111 | o       | &#111;      |           | Lowercase o                            |
| [112](https://www.ascii-code.com/112 "ASCII Code 112") | 160 | 70  | 01110000 | p       | &#112;      |           | Lowercase p                            |
| [113](https://www.ascii-code.com/113 "ASCII Code 113") | 161 | 71  | 01110001 | q       | &#113;      |           | Lowercase q                            |
| [114](https://www.ascii-code.com/114 "ASCII Code 114") | 162 | 72  | 01110010 | r       | &#114;      |           | Lowercase r                            |
| [115](https://www.ascii-code.com/115 "ASCII Code 115") | 163 | 73  | 01110011 | s       | &#115;      |           | Lowercase s                            |
| [116](https://www.ascii-code.com/116 "ASCII Code 116") | 164 | 74  | 01110100 | t       | &#116;      |           | Lowercase t                            |
| [117](https://www.ascii-code.com/117 "ASCII Code 117") | 165 | 75  | 01110101 | u       | &#117;      |           | Lowercase u                            |
| [118](https://www.ascii-code.com/118 "ASCII Code 118") | 166 | 76  | 01110110 | v       | &#118;      |           | Lowercase v                            |
| [119](https://www.ascii-code.com/119 "ASCII Code 119") | 167 | 77  | 01110111 | w       | &#119;      |           | Lowercase w                            |
| [120](https://www.ascii-code.com/120 "ASCII Code 120") | 170 | 78  | 01111000 | x       | &#120;      |           | Lowercase x                            |
| [121](https://www.ascii-code.com/121 "ASCII Code 121") | 171 | 79  | 01111001 | y       | &#121;      |           | Lowercase y                            |
| [122](https://www.ascii-code.com/122 "ASCII Code 122") | 172 | 7A  | 01111010 | z       | &#122;      |           | Lowercase z                            |
| [123](https://www.ascii-code.com/123 "ASCII Code 123") | 173 | 7B  | 01111011 | {       | &#123;      | &lcub;    | Opening brace                          |
| [124](https://www.ascii-code.com/124 "ASCII Code 124") | 174 | 7C  | 01111100 | \|      | &#124;      | &verbar;  | Vertical bar                           |
| [125](https://www.ascii-code.com/125 "ASCII Code 125") | 175 | 7D  | 01111101 | }       | &#125;      | &rcub;    | Closing brace                          |
| [126](https://www.ascii-code.com/126 "ASCII Code 126") | 176 | 7E  | 01111110 | ~       | &#126;      | &tilde;   | Equivalency sign - tilde               |
| [127](https://www.ascii-code.com/127 "ASCII Code 127") | 177 | 7F  | 01111111 | DEL     | &#127;      |           | Delete                                 |

## Extended ASCII Table
|DEC|OCT|HEX|BIN|Symbol|HTML Number|HTML Name|Description|
|---|---|---|---|---|---|---|:--|
|[128](https://www.ascii-code.com/CP1252/128 "ASCII Code 128 (Windows-1252)")|200|80|10000000|€|&#8364;|&euro;|Euro sign|
|[129](https://www.ascii-code.com/CP1252/129 "ASCII Code 129 (Windows-1252)")|201|81|10000001||||Unused|
|[130](https://www.ascii-code.com/CP1252/130 "ASCII Code 130 (Windows-1252)")|202|82|10000010|‚|&#130;|&sbquo;|Single low-9 quotation mark|
|[131](https://www.ascii-code.com/CP1252/131 "ASCII Code 131 (Windows-1252)")|203|83|10000011|ƒ|&#131;|&fnof;|Latin small letter f with hook|
|[132](https://www.ascii-code.com/CP1252/132 "ASCII Code 132 (Windows-1252)")|204|84|10000100|„|&#132;|&bdquo;|Double low-9 quotation mark|
|[133](https://www.ascii-code.com/CP1252/133 "ASCII Code 133 (Windows-1252)")|205|85|10000101|…|&#133;|&hellip;|Horizontal ellipsis|
|[134](https://www.ascii-code.com/CP1252/134 "ASCII Code 134 (Windows-1252)")|206|86|10000110|†|&#134;|&dagger;|Dagger|
|[135](https://www.ascii-code.com/CP1252/135 "ASCII Code 135 (Windows-1252)")|207|87|10000111|‡|&#135;|&Dagger;|Double dagger|
|[136](https://www.ascii-code.com/CP1252/136 "ASCII Code 136 (Windows-1252)")|210|88|10001000|ˆ|&#136;|&circ;|Modifier letter circumflex accent|
|[137](https://www.ascii-code.com/CP1252/137 "ASCII Code 137 (Windows-1252)")|211|89|10001001|‰|&#137;|&permil;|Per mille sign|
|[138](https://www.ascii-code.com/CP1252/138 "ASCII Code 138 (Windows-1252)")|212|8A|10001010|Š|&#138;|&Scaron;|Latin capital letter S with caron|
|[139](https://www.ascii-code.com/CP1252/139 "ASCII Code 139 (Windows-1252)")|213|8B|10001011|‹|&#139;|&lsaquo;|Single left-pointing angle quotation|
|[140](https://www.ascii-code.com/CP1252/140 "ASCII Code 140 (Windows-1252)")|214|8C|10001100|Œ|&#140;|&OElig;|Latin capital ligature OE|
|[141](https://www.ascii-code.com/CP1252/141 "ASCII Code 141 (Windows-1252)")|215|8D|10001101||||Unused|
|[142](https://www.ascii-code.com/CP1252/142 "ASCII Code 142 (Windows-1252)")|216|8E|10001110|Ž|&#142;|&Zcaron;|Latin capital letter Z with caron|
|[143](https://www.ascii-code.com/CP1252/143 "ASCII Code 143 (Windows-1252)")|217|8F|10001111||||Unused|
|[144](https://www.ascii-code.com/CP1252/144 "ASCII Code 144 (Windows-1252)")|220|90|10010000||||Unused|
|[145](https://www.ascii-code.com/CP1252/145 "ASCII Code 145 (Windows-1252)")|221|91|10010001|‘|&#145;|&lsquo;|Left single quotation mark|
|[146](https://www.ascii-code.com/CP1252/146 "ASCII Code 146 (Windows-1252)")|222|92|10010010|’|&#146;|&rsquo;|Right single quotation mark|
|[147](https://www.ascii-code.com/CP1252/147 "ASCII Code 147 (Windows-1252)")|223|93|10010011|“|&#147;|&ldquo;|Left double quotation mark|
|[148](https://www.ascii-code.com/CP1252/148 "ASCII Code 148 (Windows-1252)")|224|94|10010100|”|&#148;|&rdquo;|Right double quotation mark|
|[149](https://www.ascii-code.com/CP1252/149 "ASCII Code 149 (Windows-1252)")|225|95|10010101|•|&#149;|&bull;|Bullet|
|[150](https://www.ascii-code.com/CP1252/150 "ASCII Code 150 (Windows-1252)")|226|96|10010110|–|&#150;|&ndash;|En dash|
|[151](https://www.ascii-code.com/CP1252/151 "ASCII Code 151 (Windows-1252)")|227|97|10010111|—|&#151;|&mdash;|Em dash|
|[152](https://www.ascii-code.com/CP1252/152 "ASCII Code 152 (Windows-1252)")|230|98|10011000|˜|&#152;|&tilde;|Small tilde|
|[153](https://www.ascii-code.com/CP1252/153 "ASCII Code 153 (Windows-1252)")|231|99|10011001|™|&#153;|&trade;|Trade mark sign|
|[154](https://www.ascii-code.com/CP1252/154 "ASCII Code 154 (Windows-1252)")|232|9A|10011010|š|&#154;|&scaron;|Latin small letter S with caron|
|[155](https://www.ascii-code.com/CP1252/155 "ASCII Code 155 (Windows-1252)")|233|9B|10011011|›|&#155;|&rsaquo;|Single right-pointing angle quotation mark|
|[156](https://www.ascii-code.com/CP1252/156 "ASCII Code 156 (Windows-1252)")|234|9C|10011100|œ|&#156;|&oelig;|Latin small ligature oe|
|[157](https://www.ascii-code.com/CP1252/157 "ASCII Code 157 (Windows-1252)")|235|9D|10011101||||Unused|
|[158](https://www.ascii-code.com/CP1252/158 "ASCII Code 158 (Windows-1252)")|236|9E|10011110|ž|&#158;|&zcaron;|Latin small letter z with caron|
|[159](https://www.ascii-code.com/CP1252/159 "ASCII Code 159 (Windows-1252)")|237|9F|10011111|Ÿ|&#159;|&Yuml;|Latin capital letter Y with diaeresis|
|[160](https://www.ascii-code.com/CP1252/160 "ASCII Code 160 (Windows-1252)")|240|A0|10100000|NBSP|&#160;|&nbsp;|Non-breaking space|
|[161](https://www.ascii-code.com/CP1252/161 "ASCII Code 161 (Windows-1252)")|241|A1|10100001|¡|&#161;|&iexcl;|Inverted exclamation mark|
|[162](https://www.ascii-code.com/CP1252/162 "ASCII Code 162 (Windows-1252)")|242|A2|10100010|¢|&#162;|&cent;|Cent sign|
|[163](https://www.ascii-code.com/CP1252/163 "ASCII Code 163 (Windows-1252)")|243|A3|10100011|£|&#163;|&pound;|Pound sign|
|[164](https://www.ascii-code.com/CP1252/164 "ASCII Code 164 (Windows-1252)")|244|A4|10100100|¤|&#164;|&curren;|Currency sign|
|[165](https://www.ascii-code.com/CP1252/165 "ASCII Code 165 (Windows-1252)")|245|A5|10100101|¥|&#165;|&yen;|Yen sign|
|[166](https://www.ascii-code.com/CP1252/166 "ASCII Code 166 (Windows-1252)")|246|A6|10100110|¦|&#166;|&brvbar;|Pipe, broken vertical bar|
|[167](https://www.ascii-code.com/CP1252/167 "ASCII Code 167 (Windows-1252)")|247|A7|10100111|§|&#167;|&sect;|Section sign|
|[168](https://www.ascii-code.com/CP1252/168 "ASCII Code 168 (Windows-1252)")|250|A8|10101000|¨|&#168;|&uml;|Spacing diaeresis - umlaut|
|[169](https://www.ascii-code.com/CP1252/169 "ASCII Code 169 (Windows-1252)")|251|A9|10101001|©|&#169;|&copy;|Copyright sign|
|[170](https://www.ascii-code.com/CP1252/170 "ASCII Code 170 (Windows-1252)")|252|AA|10101010|ª|&#170;|&ordf;|Feminine ordinal indicator|
|[171](https://www.ascii-code.com/CP1252/171 "ASCII Code 171 (Windows-1252)")|253|AB|10101011|«|&#171;|&laquo;|Left double angle quotes|
|[172](https://www.ascii-code.com/CP1252/172 "ASCII Code 172 (Windows-1252)")|254|AC|10101100|¬|&#172;|&not;|Negation|
|[173](https://www.ascii-code.com/CP1252/173 "ASCII Code 173 (Windows-1252)")|255|AD|10101101|­SHY|&#173;|&shy;|Soft hyphen|
|[174](https://www.ascii-code.com/CP1252/174 "ASCII Code 174 (Windows-1252)")|256|AE|10101110|®|&#174;|&reg;|Registered trade mark sign|
|[175](https://www.ascii-code.com/CP1252/175 "ASCII Code 175 (Windows-1252)")|257|AF|10101111|¯|&#175;|&macr;|Spacing macron - overline|
|[176](https://www.ascii-code.com/CP1252/176 "ASCII Code 176 (Windows-1252)")|260|B0|10110000|°|&#176;|&deg;|Degree sign|
|[177](https://www.ascii-code.com/CP1252/177 "ASCII Code 177 (Windows-1252)")|261|B1|10110001|±|&#177;|&plusmn;|Plus-or-minus sign|
|[178](https://www.ascii-code.com/CP1252/178 "ASCII Code 178 (Windows-1252)")|262|B2|10110010|²|&#178;|&sup2;|Superscript two - squared|
|[179](https://www.ascii-code.com/CP1252/179 "ASCII Code 179 (Windows-1252)")|263|B3|10110011|³|&#179;|&sup3;|Superscript three - cubed|
|[180](https://www.ascii-code.com/CP1252/180 "ASCII Code 180 (Windows-1252)")|264|B4|10110100|´|&#180;|&acute;|Acute accent - spacing acute|
|[181](https://www.ascii-code.com/CP1252/181 "ASCII Code 181 (Windows-1252)")|265|B5|10110101|µ|&#181;|&micro;|Micro sign|
|[182](https://www.ascii-code.com/CP1252/182 "ASCII Code 182 (Windows-1252)")|266|B6|10110110|¶|&#182;|&para;|Pilcrow sign - paragraph sign|
|[183](https://www.ascii-code.com/CP1252/183 "ASCII Code 183 (Windows-1252)")|267|B7|10110111|·|&#183;|&middot;|Middle dot - Georgian comma|
|[184](https://www.ascii-code.com/CP1252/184 "ASCII Code 184 (Windows-1252)")|270|B8|10111000|¸|&#184;|&cedil;|Spacing cedilla|
|[185](https://www.ascii-code.com/CP1252/185 "ASCII Code 185 (Windows-1252)")|271|B9|10111001|¹|&#185;|&sup1;|Superscript one|
|[186](https://www.ascii-code.com/CP1252/186 "ASCII Code 186 (Windows-1252)")|272|BA|10111010|º|&#186;|&ordm;|Masculine ordinal indicator|
|[187](https://www.ascii-code.com/CP1252/187 "ASCII Code 187 (Windows-1252)")|273|BB|10111011|»|&#187;|&raquo;|Right double angle quotes|
|[188](https://www.ascii-code.com/CP1252/188 "ASCII Code 188 (Windows-1252)")|274|BC|10111100|¼|&#188;|&frac14;|Fraction one quarter|
|[189](https://www.ascii-code.com/CP1252/189 "ASCII Code 189 (Windows-1252)")|275|BD|10111101|½|&#189;|&frac12;|Fraction one half|
|[190](https://www.ascii-code.com/CP1252/190 "ASCII Code 190 (Windows-1252)")|276|BE|10111110|¾|&#190;|&frac34;|Fraction three quarters|
|[191](https://www.ascii-code.com/CP1252/191 "ASCII Code 191 (Windows-1252)")|277|BF|10111111|¿|&#191;|&iquest;|Inverted question mark|
|[192](https://www.ascii-code.com/CP1252/192 "ASCII Code 192 (Windows-1252)")|300|C0|11000000|À|&#192;|&Agrave;|Latin capital letter A with grave|
|[193](https://www.ascii-code.com/CP1252/193 "ASCII Code 193 (Windows-1252)")|301|C1|11000001|Á|&#193;|&Aacute;|Latin capital letter A with acute|
|[194](https://www.ascii-code.com/CP1252/194 "ASCII Code 194 (Windows-1252)")|302|C2|11000010|Â|&#194;|&Acirc;|Latin capital letter A with circumflex|
|[195](https://www.ascii-code.com/CP1252/195 "ASCII Code 195 (Windows-1252)")|303|C3|11000011|Ã|&#195;|&Atilde;|Latin capital letter A with tilde|
|[196](https://www.ascii-code.com/CP1252/196 "ASCII Code 196 (Windows-1252)")|304|C4|11000100|Ä|&#196;|&Auml;|Latin capital letter A with diaeresis|
|[197](https://www.ascii-code.com/CP1252/197 "ASCII Code 197 (Windows-1252)")|305|C5|11000101|Å|&#197;|&Aring;|Latin capital letter A with ring above|
|[198](https://www.ascii-code.com/CP1252/198 "ASCII Code 198 (Windows-1252)")|306|C6|11000110|Æ|&#198;|&AElig;|Latin capital letter AE|
|[199](https://www.ascii-code.com/CP1252/199 "ASCII Code 199 (Windows-1252)")|307|C7|11000111|Ç|&#199;|&Ccedil;|Latin capital letter C with cedilla|
|[200](https://www.ascii-code.com/CP1252/200 "ASCII Code 200 (Windows-1252)")|310|C8|11001000|È|&#200;|&Egrave;|Latin capital letter E with grave|
|[201](https://www.ascii-code.com/CP1252/201 "ASCII Code 201 (Windows-1252)")|311|C9|11001001|É|&#201;|&Eacute;|Latin capital letter E with acute|
|[202](https://www.ascii-code.com/CP1252/202 "ASCII Code 202 (Windows-1252)")|312|CA|11001010|Ê|&#202;|&Ecirc;|Latin capital letter E with circumflex|
|[203](https://www.ascii-code.com/CP1252/203 "ASCII Code 203 (Windows-1252)")|313|CB|11001011|Ë|&#203;|&Euml;|Latin capital letter E with diaeresis|
|[204](https://www.ascii-code.com/CP1252/204 "ASCII Code 204 (Windows-1252)")|314|CC|11001100|Ì|&#204;|&Igrave;|Latin capital letter I with grave|
|[205](https://www.ascii-code.com/CP1252/205 "ASCII Code 205 (Windows-1252)")|315|CD|11001101|Í|&#205;|&Iacute;|Latin capital letter I with acute|
|[206](https://www.ascii-code.com/CP1252/206 "ASCII Code 206 (Windows-1252)")|316|CE|11001110|Î|&#206;|&Icirc;|Latin capital letter I with circumflex|
|[207](https://www.ascii-code.com/CP1252/207 "ASCII Code 207 (Windows-1252)")|317|CF|11001111|Ï|&#207;|&Iuml;|Latin capital letter I with diaeresis|
|[208](https://www.ascii-code.com/CP1252/208 "ASCII Code 208 (Windows-1252)")|320|D0|11010000|Ð|&#208;|&ETH;|Latin capital letter ETH|
|[209](https://www.ascii-code.com/CP1252/209 "ASCII Code 209 (Windows-1252)")|321|D1|11010001|Ñ|&#209;|&Ntilde;|Latin capital letter N with tilde|
|[210](https://www.ascii-code.com/CP1252/210 "ASCII Code 210 (Windows-1252)")|322|D2|11010010|Ò|&#210;|&Ograve;|Latin capital letter O with grave|
|[211](https://www.ascii-code.com/CP1252/211 "ASCII Code 211 (Windows-1252)")|323|D3|11010011|Ó|&#211;|&Oacute;|Latin capital letter O with acute|
|[212](https://www.ascii-code.com/CP1252/212 "ASCII Code 212 (Windows-1252)")|324|D4|11010100|Ô|&#212;|&Ocirc;|Latin capital letter O with circumflex|
|[213](https://www.ascii-code.com/CP1252/213 "ASCII Code 213 (Windows-1252)")|325|D5|11010101|Õ|&#213;|&Otilde;|Latin capital letter O with tilde|
|[214](https://www.ascii-code.com/CP1252/214 "ASCII Code 214 (Windows-1252)")|326|D6|11010110|Ö|&#214;|&Ouml;|Latin capital letter O with diaeresis|
|[215](https://www.ascii-code.com/CP1252/215 "ASCII Code 215 (Windows-1252)")|327|D7|11010111|×|&#215;|&times;|Multiplication sign|
|[216](https://www.ascii-code.com/CP1252/216 "ASCII Code 216 (Windows-1252)")|330|D8|11011000|Ø|&#216;|&Oslash;|Latin capital letter O with slash|
|[217](https://www.ascii-code.com/CP1252/217 "ASCII Code 217 (Windows-1252)")|331|D9|11011001|Ù|&#217;|&Ugrave;|Latin capital letter U with grave|
|[218](https://www.ascii-code.com/CP1252/218 "ASCII Code 218 (Windows-1252)")|332|DA|11011010|Ú|&#218;|&Uacute;|Latin capital letter U with acute|
|[219](https://www.ascii-code.com/CP1252/219 "ASCII Code 219 (Windows-1252)")|333|DB|11011011|Û|&#219;|&Ucirc;|Latin capital letter U with circumflex|
|[220](https://www.ascii-code.com/CP1252/220 "ASCII Code 220 (Windows-1252)")|334|DC|11011100|Ü|&#220;|&Uuml;|Latin capital letter U with diaeresis|
|[221](https://www.ascii-code.com/CP1252/221 "ASCII Code 221 (Windows-1252)")|335|DD|11011101|Ý|&#221;|&Yacute;|Latin capital letter Y with acute|
|[222](https://www.ascii-code.com/CP1252/222 "ASCII Code 222 (Windows-1252)")|336|DE|11011110|Þ|&#222;|&THORN;|Latin capital letter THORN|
|[223](https://www.ascii-code.com/CP1252/223 "ASCII Code 223 (Windows-1252)")|337|DF|11011111|ß|&#223;|&szlig;|Latin small letter sharp s - ess-zed|
|[224](https://www.ascii-code.com/CP1252/224 "ASCII Code 224 (Windows-1252)")|340|E0|11100000|à|&#224;|&agrave;|Latin small letter a with grave|
|[225](https://www.ascii-code.com/CP1252/225 "ASCII Code 225 (Windows-1252)")|341|E1|11100001|á|&#225;|&aacute;|Latin small letter a with acute|
|[226](https://www.ascii-code.com/CP1252/226 "ASCII Code 226 (Windows-1252)")|342|E2|11100010|â|&#226;|&acirc;|Latin small letter a with circumflex|
|[227](https://www.ascii-code.com/CP1252/227 "ASCII Code 227 (Windows-1252)")|343|E3|11100011|ã|&#227;|&atilde;|Latin small letter a with tilde|
|[228](https://www.ascii-code.com/CP1252/228 "ASCII Code 228 (Windows-1252)")|344|E4|11100100|ä|&#228;|&auml;|Latin small letter a with diaeresis|
|[229](https://www.ascii-code.com/CP1252/229 "ASCII Code 229 (Windows-1252)")|345|E5|11100101|å|&#229;|&aring;|Latin small letter a with ring above|
|[230](https://www.ascii-code.com/CP1252/230 "ASCII Code 230 (Windows-1252)")|346|E6|11100110|æ|&#230;|&aelig;|Latin small letter ae|
|[231](https://www.ascii-code.com/CP1252/231 "ASCII Code 231 (Windows-1252)")|347|E7|11100111|ç|&#231;|&ccedil;|Latin small letter c with cedilla|
|[232](https://www.ascii-code.com/CP1252/232 "ASCII Code 232 (Windows-1252)")|350|E8|11101000|è|&#232;|&egrave;|Latin small letter e with grave|
|[233](https://www.ascii-code.com/CP1252/233 "ASCII Code 233 (Windows-1252)")|351|E9|11101001|é|&#233;|&eacute;|Latin small letter e with acute|
|[234](https://www.ascii-code.com/CP1252/234 "ASCII Code 234 (Windows-1252)")|352|EA|11101010|ê|&#234;|&ecirc;|Latin small letter e with circumflex|
|[235](https://www.ascii-code.com/CP1252/235 "ASCII Code 235 (Windows-1252)")|353|EB|11101011|ë|&#235;|&euml;|Latin small letter e with diaeresis|
|[236](https://www.ascii-code.com/CP1252/236 "ASCII Code 236 (Windows-1252)")|354|EC|11101100|ì|&#236;|&igrave;|Latin small letter i with grave|
|[237](https://www.ascii-code.com/CP1252/237 "ASCII Code 237 (Windows-1252)")|355|ED|11101101|í|&#237;|&iacute;|Latin small letter i with acute|
|[238](https://www.ascii-code.com/CP1252/238 "ASCII Code 238 (Windows-1252)")|356|EE|11101110|î|&#238;|&icirc;|Latin small letter i with circumflex|
|[239](https://www.ascii-code.com/CP1252/239 "ASCII Code 239 (Windows-1252)")|357|EF|11101111|ï|&#239;|&iuml;|Latin small letter i with diaeresis|
|[240](https://www.ascii-code.com/CP1252/240 "ASCII Code 240 (Windows-1252)")|360|F0|11110000|ð|&#240;|&eth;|Latin small letter eth|
|[241](https://www.ascii-code.com/CP1252/241 "ASCII Code 241 (Windows-1252)")|361|F1|11110001|ñ|&#241;|&ntilde;|Latin small letter n with tilde|
|[242](https://www.ascii-code.com/CP1252/242 "ASCII Code 242 (Windows-1252)")|362|F2|11110010|ò|&#242;|&ograve;|Latin small letter o with grave|
|[243](https://www.ascii-code.com/CP1252/243 "ASCII Code 243 (Windows-1252)")|363|F3|11110011|ó|&#243;|&oacute;|Latin small letter o with acute|
|[244](https://www.ascii-code.com/CP1252/244 "ASCII Code 244 (Windows-1252)")|364|F4|11110100|ô|&#244;|&ocirc;|Latin small letter o with circumflex|
|[245](https://www.ascii-code.com/CP1252/245 "ASCII Code 245 (Windows-1252)")|365|F5|11110101|õ|&#245;|&otilde;|Latin small letter o with tilde|
|[246](https://www.ascii-code.com/CP1252/246 "ASCII Code 246 (Windows-1252)")|366|F6|11110110|ö|&#246;|&ouml;|Latin small letter o with diaeresis|
|[247](https://www.ascii-code.com/CP1252/247 "ASCII Code 247 (Windows-1252)")|367|F7|11110111|÷|&#247;|&divide;|Division sign|
|[248](https://www.ascii-code.com/CP1252/248 "ASCII Code 248 (Windows-1252)")|370|F8|11111000|ø|&#248;|&oslash;|Latin small letter o with slash|
|[249](https://www.ascii-code.com/CP1252/249 "ASCII Code 249 (Windows-1252)")|371|F9|11111001|ù|&#249;|&ugrave;|Latin small letter u with grave|
|[250](https://www.ascii-code.com/CP1252/250 "ASCII Code 250 (Windows-1252)")|372|FA|11111010|ú|&#250;|&uacute;|Latin small letter u with acute|
|[251](https://www.ascii-code.com/CP1252/251 "ASCII Code 251 (Windows-1252)")|373|FB|11111011|û|&#251;|&ucirc;|Latin small letter u with circumflex|
|[252](https://www.ascii-code.com/CP1252/252 "ASCII Code 252 (Windows-1252)")|374|FC|11111100|ü|&#252;|&uuml;|Latin small letter u with diaeresis|
|[253](https://www.ascii-code.com/CP1252/253 "ASCII Code 253 (Windows-1252)")|375|FD|11111101|ý|&#253;|&yacute;|Latin small letter y with acute|
|[254](https://www.ascii-code.com/CP1252/254 "ASCII Code 254 (Windows-1252)")|376|FE|11111110|þ|&#254;|&thorn;|Latin small letter thorn|
|[255](https://www.ascii-code.com/CP1252/255 "ASCII Code 255 (Windows-1252)")|377|FF|11111111|ÿ|&#255;|&yuml;|Latin small letter y with diaeresis|

