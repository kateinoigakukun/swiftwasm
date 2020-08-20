class Base<T> {
    let v: T
    init(_ v: T) { self.v = v }
    func getValue() -> T { v }
}

private class Box<T> : Base<T> {
    override init(_ v: T) { super.init(v) }
}

private func makeBox<T>(_ v: T) -> Base<T> {
    Box(v)
}

private let box = makeBox(1)
_ = box.getValue()
