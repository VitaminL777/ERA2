int TerminalSendCommand(WORD CO, char* Data, BOOL Response) {
	return ppd_->ef.TerminalSendCommand(CO, Data, Response);
}
int TerminalSendCommandAndState(WORD CO, PERAState pEraSt, BOOL Response) {
	return ppd_->ef.TerminalSendCommandAndState(CO, pEraSt, Response);
}
int LogMsg(char *LogMessage) {
	return ppd_->ef.LogMsg1(LogMessage);
}
int LogMsg(char *LogMessage, int i) {
	return ppd_->ef.LogMsg2(LogMessage, i);
}
int LogMsg(char *LogMessage, char *str2) {
	return ppd_->ef.LogMsg3(LogMessage, str2);
}
int LogMsg(char *LogMessage, char *str2, char *str3) {
	return ppd_->ef.LogMsg4(LogMessage, str2, str3);
}
int LogMsg(char *LogMessage, char *str2, int i) {
	return ppd_->ef.LogMsg5(LogMessage, str2, i);
}
int LogMsg(char *LogMessage, int i, char *str2) {
	return ppd_->ef.LogMsg6(LogMessage, i, str2);
}
int LogMsg(char *LogMessage, int i, char *str2, int i2) {
	return ppd_->ef.LogMsg7(LogMessage, i, str2, i2);
}
int LogMsg(char *LogMessage, char *str2, char *str3, int i) {
	return ppd_->ef.LogMsg8(LogMessage, str2, str3, i);
}
int LogMsg(char *LogMessage, char *str2, int i, char *str3, int i2) {
	return ppd_->ef.LogMsg9(LogMessage, str2, i, str3, i2);
}
int LogMsg(char *LogMessage, char *str2, char *str3, char *str4) {
	return ppd_->ef.LogMsg10(LogMessage, str2, str3, str4);
}
