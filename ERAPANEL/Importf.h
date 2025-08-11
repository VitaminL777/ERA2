
int TerminalSendCommand(WORD CO, char* Data, BOOL Response=0);
int TerminalSendCommandAndState(WORD CO, PERAState pEraSt, BOOL Response = 0);
int LogMsg(char *LogMessage);
int LogMsg(char *LogMessage, int i);
int LogMsg(char *LogMessage, char *str2);
int LogMsg(char *LogMessage, char *str2, char *str3);
int LogMsg(char *LogMessage, char *str2, int i);
int LogMsg(char *LogMessage, int i, char *str2);
int LogMsg(char *LogMessage, int i, char *str2, int i2);
int LogMsg(char *LogMessage, char *str2, char *str3, int i);
int LogMsg(char *LogMessage, char *str2, int i, char *str3, int i2);
int LogMsg(char *LogMessage, char *str2, char *str3, char *str4);
