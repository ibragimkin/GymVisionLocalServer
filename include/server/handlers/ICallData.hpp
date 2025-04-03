#pragma once
class ICallData {
public:
    virtual void Proceed() = 0;
    virtual ~ICallData() = default;
};
