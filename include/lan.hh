#ifndef LAN_HH
#define LAN_HH

class Lan : public IPv4 {
  public:
    explicit Lan(char * address);

    void addNode(char * address) override;
};

#endif // LAN_HH

