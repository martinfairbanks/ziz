////////////////////////////////////
//~ NOTE: Strings

typedef struct String
{
    union
    {
        char *string;
        void *data;
    };
    u64 size;
} String;


i32 cStringLength(char *string) {
	/*int i = 0;
	while (token[i] != '\0')
		i++;

	return i;*/
    
	i32 length = 0;
	while (*string++)
		length++;
    
	return length;
    
}

// TODO: use memory arena?
String createString(i32 size)
{
    String string = {0};
    string.string = malloc(size);
    string.size = size;
    return string;
}

void freeString(String string)
{
    free(string.string);    
}

String convertCString(char *str)
{
    String string = {0};
    string.string = str;
    string.size = cStringLength(str);
    return string;
}

void cStringConcat(char *str1, char *str2, char *dest, i32 destCount)
{
    i32 str1Count = cStringLength(str1);
    i32 str2Count = cStringLength(str2);
    
    assert(str1Count+str2Count < destCount);
    
	for (i32 i = 0; i < str1Count; i++)
		*dest++ = *str1++;
    
	for (i32 i = 0; i < str2Count; i++)
		*dest++ = *str2++;
	
    *dest = 0;
}

void stringConcat(String *str1, String *str2, String *dest)
{
    assert(str1->size+str2->size <= dest->size);
	for (u32 i = 0; i < str1->size; i++)
		*dest->string++ = *str1->string++;
    
	for (u32 i = 0; i < str2->size; i++)
		*dest->string++ = *str2->string++;
	
	*dest->string = 0;
}
