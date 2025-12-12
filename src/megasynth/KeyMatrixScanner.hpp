int numColumns();
int numRows();
void setupKeyMatrixScanner();
void activateColumn(int index);
void deactivateColumn(int index);
int readRow(int index);

char scanKey();
uint32_t scanKeysMask();  
char keyFromIndex(uint8_t idx); 
