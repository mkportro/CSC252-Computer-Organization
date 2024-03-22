#include <stdio.h>
#include <stdint.h>

int check_LSB(uint8_t x);

/*
  Check if the number of bits in `word` is even.

  Returns 1 if the number of bits in the word set to 1 is even, 0
  otherwise.

  e.g., using binary representation for word:

  0101 0000 -> should return 1
  0000 0001 -> should return 0
  1111 0000 -> should return 1
*/
/*
* while word != 0
*  check least significant bit (LSB)
*   if LSB==0 then shift right 1
*   else LSB==1, shift >> 1, add count
* check count even/odd
*  return 1 for even, 0 for odd
*/

int check_even_parity(uint8_t word) {

    uint8_t zero = 0x0;
    int count = 0;

    while (word != zero){
        if (check_LSB(word) == 1) {
            word = word >> 1;
        } else {
            word = word >> 1;
            count++;
        }
    }

    if (count % 2 == 0) {
        return 1;
    } else {
        return 0;
    }
}

/*
 * Uses masking to check if least significant bit is 0.
 *
 * return 1 for true, 0 for false
 */
int check_LSB(uint8_t x) {
    if ((x & 0x01) != 0x0) { //LSB != 0
        return 0;
    } else {
        return 1;
    }
}

/*
  Set the parity bit to 1 if the number of 1 bits in the lowest 7 bits
  of `word` is odd, otherwise set it to zero.

  The parity bit is the most significant bit. Ignore the value of any
  existing parity bit.

  I.e., `check_even_parity(set_even_parity(word))` should always be 1
  for any `word`.

  e.g.:

  0101 0000 -> should return 0101 0000 (unchanged)
  0000 0001 -> should return 1000 0001 (parity bit is set to 1)
  1111 0000 -> should return 1111 0000 (unchanged,
                                        alternatively the parity bit is set)
*/
/* send word to check_even_parity
 *  if 1, word is even, do nothing
 *      return 1
 *  else 0, word is uneven, need to change bit 7 to 1
 *      if word is 128, even parity, do nothing
 *          return 1
 *      else use 128 for masking val, then OR word for new value
 *          return new value
 */
uint8_t set_even_parity(uint8_t word) {
    int val = check_even_parity(word);
    uint8_t mask = 0x80;
    uint8_t new;

    if (val == 1) {
        return 1;
    } else {
        if (word == mask) {
            return 1;
        } else {
            new = mask | word;
            return new;
        }
    }
}

/*
  Create a code word with multiple parity bits.

  `v` will always be a value between 0 and 15.

  Assuming the bits of `v` are labeled `v1`, `v2`, `v3` and `v4` from
  least to most significant, compute the parity bits as follows:

    p1 = v1 ^ v2 ^ v4
    p2 = v1 ^ v3 ^ v4
    p3 = v2 ^ v3 ^ v4

  The construct a result code word such that the parity bits and v
  bits are stored in the following bit positions:

  pos  7  6  5  4  3  2  1  0
       0 p3 p2 p1 v4 v3 v2 v1

  Note that the most significant bit is 0.

  e.g. using binary notation:

   0000 -> returns 0000 0000
   1111 -> returns 0111 1111
   0101 -> returns 0101 0101
*/
/*
 * create and assign uints for v1,v2,v3,v4
 * v1=<<7>>7
 * v2=>>1<<7>>6
 * v3=>>2<<7>>5
 * v4=>>3<<7>>4
 *
 * create and assign uints for p1,p2,p3
 * p1=(v1^v2)^v4
 * p2=(v1^v3)^4
 * p3=(v2^v3)^4
 *
 * set pb values
 *      p1bit=p1
 *      p2bit=(p2<<5)
 *      p3bit=(p2<<6)
 *
 * finalPB = 0x0 | p1 | p2 | p3
 * return finalPB
 *
 */

uint8_t create_mp_code_word(uint8_t v) {
    //lower 4 values
    uint8_t v1 = v << 7;
    v1 >>= 7;
    uint8_t v2 = v << 6;
    v2 >>= 7;
    v2 <<= 1;
    uint8_t v3 = v << 5;
    v3 >>= 7;
    v3 <<= 2;
    uint8_t v4 = v << 4;
    v4 >>= 7;
    v4 <<= 3;

    //parity bit extract
    uint8_t p1 = (v1 ^ (v2 >> 1)) ^ (v4 >> 3);
    uint8_t p2 = (v1 ^ (v3 >> 2)) ^ (v4 >> 3);
    uint8_t p3 = ((v2 >> 1) ^ (v3 >> 2)) ^ (v4 >> 3);

    //parity bit set
    uint8_t pbit1 = (p1 << 4);
    uint8_t pbit2 = (p2 << 5);
    uint8_t pbit3 = (p3 << 6);

    //final pb value
    uint8_t finalPBval = 0x0;
    finalPBval = finalPBval | v1;
    finalPBval = finalPBval | v2;
    finalPBval = finalPBval | v3;
    finalPBval = finalPBval | v4;
    finalPBval = finalPBval | pbit1;
    finalPBval = finalPBval | pbit2;
    finalPBval = finalPBval | pbit3;

    return finalPBval;
}

/*
   Check and correct single bit errors in a received code word.

   First, recompute the parity bits as above. Then compare the
   recomputed parity bits to the parity bits in the code word. If two
   or more parity bits differ, use the following table to identify the
   bit that was corrupted, and flip it.

   In the table below `X` marks a parity bit that is different from
   the recomputed version. So if recomputed parity bits `p1` and `p3`
   differ from the ones in `rcw`, flip bit `v2`.

   p1 p2 p3   Corrupt bit
   X  X       v1
   X     X    v2
      X  X    v3
   X  X  X    v4

   If only one parity bit is different, you don't have to do anything
   (think about why).

   Once `rcw` has been corrected, extract and return the lowest 4 bits
   (i.e. a number from 0 to 15).


  e.g.:
   0000 0000 -> 0000
   0111 1111 -> 1111
   0101 0101 -> 0101
   0101 0111 -> 0101  [note single bit error and correction]
   0111 0101 -> 0101  [note single bit error in parity bits]
*/
/*
 * extract lower 4 bits
 * send rcw to create mp code word
 * check rcmptRCW with rcw
 * counter var to keep track of incorrect parity bits
 *  if more than 2, switch correct v variable
 */
uint8_t decode(uint8_t rcw) {
    uint8_t low4 = (rcw <<4); //extract lower 4 bits
    low4 >>= 4;
    uint8_t rcmptRCW = create_mp_code_word(low4); // recompute mp code word
    //printf("low4 = %i, rcmptRCW = %i\n", low4, rcmptRCW);

    int count = 0;
    int parityPlace [3];

    uint8_t temp1;
    uint8_t temp2;

    for (int i = 3, j = 0; i > 0; i--, j++) { //check parity bits for errors
        temp1 = rcw;
        temp2 = rcmptRCW;

        temp1 <<= i;
        temp1 >>= 7;
        temp2 <<= i;
        temp2 >>= 7;

        if (temp1 != temp2) {
            count++;
            parityPlace[j] = i;
        }
    }

    //printf("count = %i\n", count);

    int p1 = 0;
    int p2 = 0;
    int p3 = 0;
    int corruptPB = 0;
    if (count >= 2) { //more than 1 PB is corrupt
        for (int i = 0; i < 4; i++) { //loop thru parity array
            if (parityPlace[i] == 3) {
                p1 = 1;
                //printf("parityPlace[%i] = %i\n", i, p1);
            } else if (parityPlace[i] == 2) {
                p2 = 1;
                //printf("parityPlace[%i] = %i\n", i, p2);
            } else if (parityPlace[i] == 3) {
                p3 = 1;
                //printf("parityPlace[%i] = %i\n", i, p3);
            }
        }
        //find which parity bits are corrupt
        if (p1 == 1 && p2 == 1) {
            corruptPB = 1;
            //printf("corruptPB = %i\n", corruptPB);
        } else if (p1 == 1 && p3 == 1) {
            corruptPB = 2;
            //printf("corruptPB = %i\n", corruptPB);
        } else if (p2 == 1 && p3 == 1) {
            corruptPB = 3;
            //printf("corruptPB = %i\n", corruptPB);
        } else if (p1 == 1 && p2 == 1 && p3 == 1) {
            corruptPB = 4;
            //printf("corruptPB = %i\n", corruptPB);
        }

        //mask and flip the corrupt bit
        uint8_t bitToFlip = 0x0;
        if (corruptPB == 1) {
            bitToFlip = 0x1;
            rcmptRCW = rcmptRCW & bitToFlip;
            rcmptRCW <<= 4;
            rcmptRCW >>= 4;
            return rcmptRCW;
        } else if (corruptPB == 2) {
            bitToFlip = 0x2;
            rcmptRCW = rcmptRCW & bitToFlip;
            rcmptRCW <<= 4;
            rcmptRCW >>= 4;
            return rcmptRCW;
        } else if (corruptPB == 3) {
            bitToFlip = 0x4;
            rcmptRCW = rcmptRCW & bitToFlip;
            rcmptRCW <<= 4;
            rcmptRCW >>= 4;
            return rcmptRCW;
        } else if (corruptPB == 4) {
            bitToFlip = 0x8;
            rcmptRCW = rcmptRCW & bitToFlip;
            rcmptRCW <<= 4;
            rcmptRCW >>= 4;
            return rcmptRCW;
        }
    }

    return rcmptRCW;
}


/*
  Create a code word that can detect double bit errors as well as
  correct single bit errors.

  This is achieved by first using multiple parity bits as described
  above and then using the most significant bit as parity check for
  the entire code word.

  Use even parity. As before, `v` is a value from 0 to 15. The return
  value should be encoded as follows:

  pos  7  6  5  4  3  2  1  0
       p p3 p2 p1 v4 v3 v2 v1

  Where `p` is parity bit for the whole code word.

  e.g.:
   0000 -> returns 0000 0000
   1111 -> returns 1111 1111
   0101 -> returns 0101 0101
 */

uint8_t create_secded_code_word(uint8_t v) {
    uint8_t mask = 0x00;

    mask = (v << 4);
    mask = (mask | v);

    return mask;
}

/*
   Check and correct single bit errors in a received code word, while
   also detecting double bit errors.

   Return a value between 0 to 15 if no double bit errors were
   detected, correcting any single bit errors as described above.

   Return 255 if a double bit error was detected.

   e.g.:

   0000 0000 -> 0000
   1111 1111 -> 1111
   0101 0101 -> 0101
   0101 0111 -> 0101       [single bit error]
   1101 1101 -> 1111 1111  [double bit error]
*/

uint8_t decode_secded(uint8_t rcw) {
    return 0;
}


/* THE MAIN AND TEST FUNCTIONS BELOW ARE FOR YOUR CONVENIENCE AND ARE
   NOT PART OF THIS ASSIGNMENT. NOTE THE TESTS ARE NOT EXHAUSTIVE.
*/

void check_equality(uint8_t a, uint8_t b, const char *fn) {
    printf("%s: %s: Checking %u == %u\n", a == b ? "PASS" : "FAIL", fn, a, b);
}

void check_equality_hex(uint8_t a, uint8_t b, const char *fn) {
    printf("%s: %s: Checking 0x%x == 0x%x\n", a == b ? "PASS" : "FAIL", fn, a, b);
}

void test_check_even_parity() {
    uint8_t v;

    check_equality(check_even_parity(0x0), 1, "check_even_parity(0x00) // 0000 0000");
    check_equality(check_even_parity(0x02), 0, "check_even_parity(0x02) // 0000 0010");
    check_equality(check_even_parity(0x06), 1, "check_even_parity(0x06) // 0000 0110");
    check_equality(check_even_parity(0x1f), 0, "check_even_parity(0x1f) // 0001 1111");
    check_equality(check_even_parity(0x69), 1, "check_even_parity(0x69) // 0110 1001");
    check_equality(check_even_parity(0x80), 0, "check_even_parity(0x80) // 1000 0000");
}

void test_set_even_parity() {
    uint8_t v;
    const uint8_t PB = 0x80;

    v = set_even_parity(0x0);
    check_equality((v & PB) >> 7, 0, "set_even_parity(0x00) / 000 / 0000 0000");

    v = set_even_parity(0x01);
    //printf("word = 1; val = %i\n", v);
    check_equality((v & PB) >> 7, 1, "set_even_parity(0x01) / 001 / 0000 0001");

    v = set_even_parity(0x1f);
    check_equality((v & PB) >> 7, 1, "set_even_parity(0x1f) / 031 / 0001 1111");

    v = set_even_parity(0x50);
    check_equality((v & PB) >> 7, 0, "set_even_parity(0x50) / 080 / 0101 0000");

    v = set_even_parity(0x70);
    check_equality((v & PB) >> 7, 1, "set_even_parity(0x70) / 112 / 0111 0000");

    v = set_even_parity(0x80);
    //printf("word = 128, val = %i\n", v);
    check_equality((v & PB) >> 7, 0, "set_even_parity(0x80) / 128 / 1000 0000");

    v = set_even_parity(0xF0);
    check_equality((v & PB) >> 7, 0, "set_even_parity(0xF0) / 240 / 1111 0000");
}

void test_create_mp_code_word() {
    uint8_t v;

    v = create_mp_code_word(0);
    check_equality_hex(v, 0, "create_mp_code_word(000) / 000 / 0000 0000");

    v = create_mp_code_word(15);
    check_equality_hex(v, 0x7f, "create_mp_code_word(015) / 015 / 0000 1111 -> 127 / 0111 1111");

    v = create_mp_code_word(5);
    check_equality_hex(v, 0x55, "create_mp_code_word(005) / 005 / 0000 0101 -> 85 / 0101 0101");

    v = create_mp_code_word(125);
    check_equality_hex(v, 0x2d, "create_mp_code_word(125) / 125 / 0111 1101 -> 0010 1101");

}

void test_decode() {
    uint8_t cw;

    cw = 0x7f;    // no errors
    check_equality(decode(cw), 15, "decode(0x7f) / 127 / 0111 1111");

    cw = 0x7f & ~0x2; // error in one value bit
    check_equality(decode(cw), 15, "decode(0x7d) / 125 / 0111 1101");

    cw = 0x7f & ~0x1; // error in one value bit
    check_equality(decode(cw), 15, "decode(0x7e) / 126 / 0111 1110");

    cw = 0x7f & ~0x10; // error in one parity bit
    check_equality(decode(cw), 15, "decode(0x6f) / 111 / 0110 1111");
}

void test_create_secded_code_word() {
    uint8_t v;

    v = create_secded_code_word(0);
    check_equality_hex(v, 0, "create_secded_code_word(00) / 0000 0000");

    v = create_secded_code_word(5);
    check_equality_hex(v, 0x55, "create_secded_code_word(05) / 0000 0101 -> 0101 0101");

    v = create_secded_code_word(7);
    check_equality_hex(v, 0x77, "create_secded_code_word(07) / 0000 0111 -> 0111 0111");

    v = create_secded_code_word(14);
    check_equality_hex(v, 0xee, "create_secded_code_word(14) / 0000 1110 -> 1110 1110");

    v = create_secded_code_word(15);
    check_equality_hex(v, 0xff, "create_secded_code_word(15) / 0000 1111 -> 1111 1111");
}

void test_decode_secded() {
    uint8_t v;

    v = decode_secded(0);
    check_equality_hex(v, 0, "decode_secded(0x00)");

    v = decode_secded(0xff);
    check_equality_hex(v, 15, "decode_secded(0xff)");

    v = decode_secded(0xff & ~0x1); // flip one bit
    check_equality_hex(v, 15, "decode_secded(0xff)");

    v = decode_secded(0xff & ~0x3); // flip two bits
    check_equality_hex(v, 255, "decode_secded(0xfc)");

    v = decode_secded(0xdd); // double bit error
    check_equality_hex(v, 255, "decode_secded(0xdd)");

}

int main(void) {
    //printf("---------------------------------------------------------------\n");
    //printf("-----------------------START TESTING---------------------------\n");
    //printf("---------------------------------------------------------------\n");
    test_check_even_parity();
    //int test = check_even_parity(set_even_parity(0x00));
    //printf("test = %i", test);
    //printf("-------------------------------------------------------------\n");
    test_set_even_parity();
    //printf("---------------------------------------------------------------\n");
    test_create_mp_code_word();
    //printf("---------------------------------------------------------------\n");
    test_decode();
    //printf("---------------------------------------------------------------\n");
    test_create_secded_code_word();
    //printf("---------------------------------------------------------------\n");
    test_decode_secded();
    //printf("---------------------------------------------------------------\n");
    //printf("------------------------END TESTING----------------------------\n");
    //printf("---------------------------------------------------------------\n");
}