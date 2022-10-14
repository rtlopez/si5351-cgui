#pragma once

#include <string.h>

class Cli
{
  public:
    typedef bool (*func_t)(const char** args);
    Cli(func_t* funcs, func_t unkn): _funcs(funcs), _unkn(unkn)
    {
      reset();
    }

    void parse(int c)
    {
      // enter pressed, process command
      if(c == '\r' || c == '\n') {
        if(_idx == 0) return; // empty buffer
        _buff[_idx] = 0;
        _idx = 0;
        reset();
        tokenize();
        process();
        return;
      }

      // ignore too long command
      if(_idx >= BUFF_SIZE - 1) return;

      // add to buffer
      _buff[_idx] = c;
      _idx++;
    }

    static bool is(const char** args, const char * cmd)
    {
      return args && cmd && strcmp(args[0], cmd) == 0;
    }
  private:
    void tokenize()
    {
      static const char * delim = " \t";
      char * pch = strtok(_buff, delim);
      size_t count = 0;
      while(pch)
      {
        _args[count++] = pch;
        pch = strtok(NULL, delim);
      }
    }

    void process()
    {
      if(!_args[0]) return;
      
      bool processed = false;
      func_t * f = _funcs;
      while(*f)
      {
        // call every command until any of them return true
        processed = (*f)(_args);
        if(processed) break;
        f++;
      }
      // none of commands processed, call unknown
      if(!processed) (_unkn)(_args);
    }

    void reset()
    {
      for(size_t i = 0; i < ARGS_SIZE; ++i) {
        _args[i] = nullptr;
      }
    }

    func_t * _funcs;
    func_t _unkn;

    static const size_t BUFF_SIZE = 64;
    static const size_t ARGS_SIZE = 8;
    size_t _idx = 0;
    char _buff[BUFF_SIZE];
    const char * _args[ARGS_SIZE];
};
