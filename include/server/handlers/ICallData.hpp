// Базовый класс для общих вещей (по желанию)
class ICallData {
public:
    virtual void Proceed() = 0;
    virtual ~ICallData() = default;
};