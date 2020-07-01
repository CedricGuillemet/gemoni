#include "Utils.h"
#include <vector>
#include <stdarg.h>

static std::vector<LogOutput> outputs;
void AddLogOutput(LogOutput output)
{
    outputs.push_back(output);
}

int Log(const char* szFormat, ...)
{
    va_list ptr_arg;
    va_start(ptr_arg, szFormat);

    static char buf[102400];
    vsprintf(buf, szFormat, ptr_arg);

    static FILE* fp = fopen("log.txt", "wt");
    if (fp)
    {
        fprintf(fp, "%s", buf);
        fflush(fp);
    }
    for (auto output : outputs)
        output(buf);
    va_end(ptr_arg);
    return 0;
}
