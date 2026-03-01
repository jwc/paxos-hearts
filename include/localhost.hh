#ifndef LOCALHOST_HH
#define LOCALHOST_HH

class Localhost : public IPv4 {
  public:
    explicit Localhost(char * port);

    void addNode(char * port) override;
};

#endif // LOCALHOST_HH

