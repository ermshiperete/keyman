class TestEnvironment {
  const char* _currentExecutable;
  int _ibus_pid;
  int _keyman_pid;

  void LaunchIbus();
  void LaunchIbusKeyman();

public:
  TestEnvironment(const char* currentExecutable);
  void Setup(const char* directory, int nKeyboards, char* keyboards[]);
  void Restore();
};
