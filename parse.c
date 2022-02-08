#include <stdio.h>
#include <stdlib.h>

struct charStack {
    char * string;
    int stringLen;
    char * ending;
    int endingLen;
    int instances;
    void * next;
};

struct str {
    char * content;
    int length;
    char isEnd;
};

char * pop(struct charStack **s) {
    char *temp1 = (*s)->string;
    // int temp2 = s->instances;
    struct charStack * stemp = (*s)->next;
    free((*s));
    *s = stemp;
    return temp1;
}

void deStack(struct charStack **s) {
    struct charStack * stemp = (*s)->next;
    free((*s));
    *s = stemp;
    return;
}

void add(struct charStack **s, char *newLoad, int newLoadLen, char* newEnd, int newEndLen, int newInstances) {
    struct charStack * newHead = malloc(sizeof(struct charStack));
    newHead->string=newLoad;
    newHead->stringLen=newLoadLen;
    newHead->ending=newEnd;
    newHead->endingLen=newEndLen;
    newHead->instances=newInstances;
    newHead->next=&(**s);
    *s= newHead;
    return;
}

struct str * consume(struct charStack **s) {
    --((*s)->instances);
    struct str * ret = malloc(sizeof(struct str));
    if((*s)->instances == 0) { // intentional : -1 instances causes 'infinite' possible uses
        ret->content=(*s)->ending;
        ret->length=(*s)->endingLen;
        ret->isEnd=1;
        deStack(s);
    }
    else {
        ret->content=(*s)->string;
        ret->length=(*s)->stringLen;
        ret->isEnd=0;
    }

    return ret;
}

struct intStack {
    int val;
    void * next;
};

void stack(struct intStack **s, int newVal) {
    struct intStack * newHead = malloc(sizeof(struct intStack));
    newHead->val=newVal;
    newHead->next=&(**s);
    *s = newHead;
    return;
}
void deStackInt(struct intStack **s) {
    struct intStack * stemp = (*s)->next;
    free((*s));
    *s = stemp;
    return;
}





int ipow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }
    return result;
}



unsigned char * inToBin(char * input, int len) {
    unsigned char* datastream = malloc(len/2 * sizeof(char));
    int ind = 0;
    char left = 1;
    for(int i=0;i<len;++i) {
        if(47<input[i] && input[i]<58) {
            if(left) {datastream[ind] = (char) ((input[i]-48)<<4);}
            else {datastream[ind] += (char) (input[i]-48);}
        }
        else if(64<input[i] && input[i]<71) {
            if(left) {datastream[ind] = (char) ((input[i]-55)<<4);}
            else {datastream[ind] += (char) (input[i]-55);}
        }
        else if(96<input[i] && input[i]<103) {
            if(left) {datastream[ind] = (char) ((input[i]-87)<<4);}
            else {datastream[ind] += (char) (input[i]-87);}
        }
        left = !left;
        if (left) {++ind;}
    }
    return datastream;
}

char * bin(unsigned char in) {
    // printf("[bin] : %c\n", in);
    static char r[9];
    int c=0;
    for(unsigned char i=128;i>0;i>>=1) {
        r[c]=  (char) ((unsigned char) 48 + ((in&i)>0));
        ++c;
    }
    r[8]=0;
    return r;
}

unsigned char intLen(unsigned char len, unsigned char * stream, int index) {
    unsigned long int longInt=0;
    for(char i=0;i<(1<<(len-24));++i) {
        ++index;
        longInt = (longInt<<8) + (unsigned long int) stream[index];
    }
    int comp=1;
    for(char i=1; i<21;++i) {
        comp=ipow(10,i);
        if(longInt<comp) {
            return i;
        }
    }
    return 0;
}

char * binToCBOR_S2S(unsigned char * inStream, int streamLen) {
    printf("[start of conversion]\n");
    char * ret;
    // first iteration to evaluate the needed output length
    int estLen=0;
    //unsigned char intLen[5] = {0,3,5,10,20};
    printf("  [estimating length of final string]\n");
    for(int byteInd=0; byteInd<streamLen; ++byteInd) {
        // integers : we can read int size and convert :
            // uint64 > 20 chars
            // uint32 > 10 chars
            // uint16 > 5 chars
            // uint8 > 3 chars
        // negative integers : same + 1 for the '-'
        // byte strings will be represented as '0b[........]n' > 2+ 8*n chars
        // text string  will take 1 char per char +2 (for '"') (assuming we can print UTF-8)
        // arrays will take 2 chars + 2 per element (in addition to what's inside)
        // maps will take 2 chars + 5 per element (', ' and ' : ') (in addition to what's inside)
        // tags will be ignored [SIMPLIFIED]
        // simple types will be considered as taking 0 char [should be ok]
        unsigned char type = ((inStream[byteInd]&224)>>5);
        unsigned char len = (inStream[byteInd]&31);
        int realLen=len;
        if (type <2) {
            if (len<24) {
                printf("    [bintoCBOR] short int\n");
                realLen = (len<10) ? 1 : 2;
            }
            else {
                printf("    [bintoCBOR] long int\n");
                realLen=intLen(len, inStream, byteInd);// inefficient but tight
                byteInd+=(1<<(len-24));
            }
        }
        else if(type>1 && len>24) {// only for not integers
            // then read next bytes for the real length
            printf("    [bintoCBOR] long length for complex type\n");
            int bylenOfLen = len-23;
            realLen=0;
            for(int foo=0; foo< bylenOfLen; ++foo) {
                ++byteInd;
                if(byteInd > streamLen) {
                    printf("    [binToCBOR_S2S] CRITICAL FAIL\n");
                    ret = malloc(sizeof(char));
                    ret[0]=0; // null string : 0 chars before end
                    return ret;
                }
                realLen = (realLen<<8)+inStream[byteInd];
            }
        }
        if(type == 0) {estLen += realLen;}
        else if(type == 1) {estLen += realLen+1;}
        else if(type == 2) {estLen += realLen*8+(realLen-1)+4;}
        else if(type == 3) {estLen += realLen+2;}
        else if(type == 4) {estLen += (realLen-1)*2+2;}
        else if(type == 5) {estLen += realLen*3+(realLen-1)*2+2;}
        else if(type == 6) {estLen += 0;}
        else if(type == 7) {estLen += 0;}
        if(type==2 || type==3) {
            // skip data bytes
            byteInd+=realLen;
        }
        printf("    (type %d , len %d) -> %d\n", type, realLen, estLen);
    }
    // With this, estLen should be roughtly the final string length (or above !)
    ret = malloc((estLen+1)*sizeof(char)); // so we're sure to have enough room for a final null byte
    ret[estLen]=0; // null-terminate it
    //fill with spaces to ensure rendering
    for(int foo=0;foo<estLen;++foo) {
        ret[foo]=32;
    }
    printf("  [estimated length : %d]\n", estLen);
    // WRITING CURSOR
    int cursor = 0;
    // INIT CHAR STACK
    struct charStack *myCharStack = malloc(sizeof(struct charStack));
    myCharStack->string = "";
    myCharStack->stringLen = 0;
    myCharStack->instances = (int) -1;
    // INIT SCOPE MAP LEN STACK
    struct intStack *myMapLenStack = malloc(sizeof(struct intStack));
    myMapLenStack->val=0;
    myMapLenStack->next=myMapLenStack;

    printf("  [writing cbor string]\n");
    for(int byteInd=0; byteInd<streamLen; ++byteInd) {
        unsigned char type = ((inStream[byteInd]&224)>>5);
        unsigned char len = (inStream[byteInd]&31);
        int realLen=len;
        if (type <2) {
            if (len<24) {realLen = (len<10) ? 1 : 2;}
            else {realLen=intLen(len, inStream, byteInd);} // inefficient but tight
        }
        else if(type>1 && len>23) {// only for not integers
            // then read next bytes for the real length
            int bylenOfLen = len-23;
            realLen=0;
            for(int foo=0; foo< bylenOfLen; ++foo) {
                ++byteInd;
                if(byteInd > streamLen) {
                    printf("    [binToCBOR_S2S] CRITICAL FAIL\n");
                    ret = malloc(sizeof(char));
                    ret[0]=0; // null string : 0 chars before end
                    return ret;
                }
                realLen = (realLen<<8)+inStream[byteInd];
            }
        }


        if((myMapLenStack->val)>0) {
            printf("    currently in map :");
            --(myMapLenStack->val);
            if ((myMapLenStack->val)&1) {
                add(&myCharStack, " : ", 3, "", 0, 2); // saves introducing an artificial couple type
                printf("key (%d)", (myMapLenStack->val));
            }
            else {
                printf("val (%d)", (myMapLenStack->val));
            }
            printf("\n");
        }
        if ((myMapLenStack->val)==0) {
            deStackInt(&myMapLenStack);
        }

        printf("    reading byte %d, type %d (%d)\n", byteInd, type, len);

        if(type==0 || type==1) {
            if(type==1) {
                ret[cursor]='-';
                ++cursor;
            }
            if(len<24){
                printf("    writing short int at %d\n", cursor);
                for(char i=0; i<realLen;++i) {
                    ret[cursor]=48+(len%(ipow(10,(realLen-i)))/ipow(10,(realLen-i-1)));
                    ++cursor;
                }
            }
            else {
                // read with len-23 level of int
                // declare max
                printf("    writing long int at %d\n", cursor);
                unsigned long int longInt=0;
                for(char i=0;i<(1<<(len-24));++i) {
                    ++byteInd;
                    longInt = (longInt<<8) + (unsigned long int) inStream[byteInd];
                }
                printf("      int is %lu (%d bytes long)\n", longInt, (1<<(len-24)));
                for(char i=0; i<realLen;++i) {
                    // printf("      writing digit '%c'\n", ((char) 48+(longInt%(ipow(10,(realLen-i)))/ipow(10,(realLen-i-1))) ) );
                    ret[cursor]=48+(longInt%(ipow(10,(realLen-i)))/ipow(10,(realLen-i-1)));
                    ++cursor;
                }
            }
        }
        else if(type == 2) {
            printf("    writing byte string of length %d bytes\n", realLen);
            ret[cursor]='\'';++cursor;
            ret[cursor]='0';++cursor;
            ret[cursor]='b';++cursor;
            for(int i=0;i<realLen;++i) {
                if(i>0) {
                    ret[cursor]=' ';
                    ++cursor;
                }
                for(int foo=7;foo>=0;--foo) {
                    ret[cursor]=48+((inStream[byteInd]>>foo)&1);
                    ++cursor;
                }
                ++byteInd;
            }
            ret[cursor]='\'';++cursor;
        }
        else if(type == 3) {
            printf("    writing string of length %d\n", realLen);
            printf("      inserting start delimiter '\"'\n");
            ret[cursor]='"';++cursor;
            add(&myCharStack, "", 0, "\"", 1, realLen);
            ++byteInd;
            int ind;
            for(ind=byteInd;ind<byteInd+realLen;++ind) {
                printf("      printing char %c\n", inStream[ind]);
                ret[cursor]=inStream[ind];
                ++cursor;
                struct str *insert = consume(&myCharStack);
                for(int foo=0;foo<(insert->length);++foo) {
                    printf("      inserting spacer/end '%c' (len:%d)\n", insert->content[foo], insert->length);
                    ret[cursor]=(insert->content)[foo];
                    ++cursor;
                }
            }
            byteInd=ind-1;
        }
        else if(type == 4) {
            printf("    writing array of length %d\n", realLen);
            printf("      inserting start delimiter\n");
            for(int i=0;i<realLen;++i) {
                stack(&myMapLenStack, 0);
            }
            ret[cursor]='[';++cursor;
            add(&myCharStack, ", ", 2, "]", 1, realLen);
            continue;
        }
        else if(type == 5) {
            printf("    writing map of length %d\n", realLen);
            stack(&myMapLenStack, realLen*2);
            printf("      inserting start delimiter\n");
            ret[cursor]='{';++cursor;
            add(&myCharStack, ", ", 2, "}", 1, realLen);
            continue;
        }
        else if(type == 6) {
            // fuck type 6
        }
        else if(type == 7) {
            // do nothing
        }
        struct str *insert = consume(&myCharStack);
        for(int foo=0;foo<(insert->length);++foo) {
            printf("  inserting spacer/end '%c' (len:%d)\n", insert->content[foo], insert->length);
            ret[cursor]=(insert->content)[foo];
            ++cursor;
        }
        while (insert->isEnd && cursor<estLen) {
            insert = consume(&myCharStack);
            for(int foo=0;foo<(insert->length) && cursor<estLen;++foo) {
                printf("  inserting spacer/end '%c' (len:%d)\n", insert->content[foo], insert->length);
                ret[cursor]=(insert->content)[foo];
                ++cursor;
            }
        }
    }
    printf("[end of conversion]\n");
    return ret;
}






void foo() {
    struct charStack * myCharStack = malloc(sizeof(struct charStack));
    myCharStack->string = " ";
    myCharStack->stringLen = 1;
    myCharStack->ending = "";
    myCharStack->endingLen = 0;
    myCharStack->instances = -1;
    myCharStack->next = myCharStack;

    add(&myCharStack, "", 0, "\"", 1, 8);
    struct str * holder;
    for(int foo=0; foo<10; ++foo) {
        printf("  (i=%d) left : %d", foo, myCharStack->instances);
        holder = consume(&myCharStack);
        printf("  \"");
        for(int i=0;i<holder->length;++i) {
            printf("%c", holder->content[i]);
        }
        printf("\"\n");
    }
}




int main(int argc, char * argv[]) {
    int len=0;
    while(argv[1][++len]);
    // printf("len : %d\n", len);
    unsigned char * myDataStream = inToBin(argv[1], len);
    // DEBUG PRINT
    printf("datastream : [");
    for(int a=0; a<len/2;++a) {
        printf("%s", bin(myDataStream[a]));
        if(a<(len/2-1)) printf(" ");
    }
    printf("]\n");
    // END OF DEBUG PRINT

    //foo();

    char * cborString = binToCBOR_S2S(myDataStream, len/2);
    printf("CBOR : <%s>\n", cborString);
    free(cborString);
    free(myDataStream);
    return 0;
}