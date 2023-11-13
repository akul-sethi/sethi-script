from typing import Any


class Func:
    def __init__(self, func) -> None:
        self.func = func
    def __call__(self, *args: Any, **kwds: Any) -> Any:
        return self.func(*args, **kwds)
    
    def __add__(self, other):
        return Func((lambda *args: self(*other(*args))))
    

square = Func(lambda x: pow(x, 2))
double = square + square
print(square(4))
print(double(4))

def test(*args, **kwds):
    print(*args)


