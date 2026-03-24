#ifndef PRINTABLE_HH
#define PRINTABLE_HH

class Printable {
  private:
    inline static int initialized = 0;
    inline static std::mutex initLock;

  protected:
    inline int useUnicode() { return supportsUnicode && preferUnicode; }

    inline int useColor() { return supportsColor && preferColor; }

  public:
    inline static int supportsUnicode, preferUnicode;
    inline static int sizeX;
    inline static int sizeY;
    inline static int supportsColor, preferColor;
  
    Printable() {
      initLock.lock();
      if ( ! initialized) {
        initialized = 1;

        
        if (setlocale(LC_ALL, "") != NULL)
          supportsUnicode = 1;
        else 
          supportsUnicode = 0;
        preferUnicode = 1;
        
        initscr();
        cbreak();
        noecho();
        nl();
        keypad(stdscr, TRUE);
        curs_set(0);
        
        mvprintw(30, 2, "Printable initialized\n");

        getmaxyx(stdscr, sizeY, sizeX);

        if (has_colors()) {
          supportsColor = 1;
          start_color();
          init_pair(1, COLOR_RED, COLOR_BLACK);
          init_pair(2, COLOR_WHITE, COLOR_BLACK);
        } else 
          supportsColor = 0;
        preferColor = 1;
      }
      initLock.unlock();
    }

    void toggleUnicode() { preferUnicode ^= 1; } 

    void toggleColor() { preferColor ^= 1; } 

    void clear() { ::clear(); getmaxyx(stdscr, sizeY, sizeX); }

    void refresh() { ::refresh(); }

    void end() {
      initLock.lock();
      endwin();
      initLock.unlock();
    }
};

#endif

