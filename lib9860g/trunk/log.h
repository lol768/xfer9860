#ifndef LOG_H
#define LOG_H

extern char fx_messageString[512];
extern unsigned int fx_debugLevel;

#define FX_LOG(level, format, args...) \
	do {	\
		snprintf(fx_messageString, sizeof(fx_messageString)-1, format, ##args);	\
		if (level <= fx_debugLevel)	\
			fprintf(stderr, "DBG(%i): %s\n", level, fx_messageString);	\
		fflush(stderr);	\
	} while(0);

#endif /* LOG_H */
