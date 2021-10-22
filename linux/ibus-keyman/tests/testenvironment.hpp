class TestEnvironment {
  const char* _currentExecutable;

  void LaunchIbus();
  void LaunchIbusKeyman();

public:
  TestEnvironment(const char* currentExecutable);
  void Setup(const char* directory, int nKeyboards, char* keyboards[]);
  void Restore();
};
