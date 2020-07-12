
template<class T, class F>
class AutoDeleter {
private:
	T Object;
	F Deleter;
	VkDevice Device;

	AutoDeleter& operator=(AutoDeleter&& other) {
		if (this != other) {
			Object = other.Object;
			Deleter = other.Deleter;
			Device = other.Device;
			other = VK_NULL_HANDLE;
		}
		return *this;
	}

public:
	AutoDeleter() :
		Object(VK_NULL_HANDLE),
		Deleter(nullptr),
		Device(VK_NULL_HANDLE) {

	}

	AutoDeleter(T object, F deleter, VkDevice device) :
		Object(object),
		Deleter(deleter),
		Device(device) {

	}

	AutoDeleter(AutoDeleter&& other) {
		*this = std::move(other);
	}

	~AutoDeleter() {
		if ((Object != VK_NULL_HANDLE) && (Deleter != nullptr) && (Device != VK_NULL_HANDLE)) {
			Deleter(Device, Object, nullptr);
		}
	}

	T Get() {
		return Object;
	}

	bool operator!() const {
		return Object == VK_NULL_HANDLE;
	}

};