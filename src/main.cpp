#include <CL/cl.h>
#include <libclew/ocl_init.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

template<typename T>
std::string to_string(T value)
{
	std::ostringstream ss;
	ss << value;
	return ss.str();
}

void reportError(cl_int err, const std::string &filename, int line)
{
	if(CL_SUCCESS == err)
		return;

	// Таблица с кодами ошибок:
	// libs/clew/CL/cl.h:178
	// P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с номером строки: cl.h:103) -> Enter
	std::string message = "OpenCL error code " + to_string(err) + " encountered at " + filename + ":" + to_string(line);
	throw std::runtime_error(message);
}

#define OCL_SAFE_CALL(expr) reportError(expr, __FILE__, __LINE__)

std::vector<char> PlatformInfo(cl_platform_id platform, cl_platform_info param_name)
{
	size_t platformParamSize = 0;
	OCL_SAFE_CALL(clGetPlatformInfo(platform, param_name, 0, nullptr, &platformParamSize));
	std::vector<char> platformParam(platformParamSize, 0);
	OCL_SAFE_CALL(clGetPlatformInfo(platform, param_name, platformParamSize, &platformParam[0], nullptr));
	return platformParam;
}

std::vector<char> DeviceInfo(cl_device_id device, cl_device_info param_name)
{
	size_t deviceParamSize = 0;
	clGetDeviceInfo(device, param_name, 0, nullptr, &deviceParamSize);
	std::vector<char> deviceParam(deviceParamSize);
	clGetDeviceInfo(device, param_name, deviceParamSize, deviceParam.data(), nullptr);
	return deviceParam;
}
int main()
{
	// Пытаемся слинковаться с символами OpenCL API в runtime (через библиотеку libs/clew)
	if(!ocl_init())
		throw std::runtime_error("Can't init OpenCL driver!");

	// Откройте
	// https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/
	// Нажмите слева: "OpenCL Runtime" -> "Query Platform Info" -> "clGetPlatformIDs"
	// Прочитайте документацию clGetPlatformIDs и убедитесь, что этот способ узнать, сколько есть платформ, соответствует документации:
	cl_uint platformsCount = 0;
	OCL_SAFE_CALL(clGetPlatformIDs(0, nullptr, &platformsCount));
	std::cout << "Number of OpenCL platforms: " << platformsCount << std::endl;

	// Тот же метод используется для того, чтобы получить идентификаторы всех платформ - сверьтесь с документацией, что это сделано верно:
	std::vector<cl_platform_id> platforms(platformsCount);
	OCL_SAFE_CALL(clGetPlatformIDs(platformsCount, platforms.data(), nullptr));

	for(int platformIndex = 0; platformIndex < platformsCount; ++platformIndex)
	{
		std::cout << "Platform #" << (platformIndex + 1) << "/" << platformsCount << std::endl;
		cl_platform_id platform = platforms[platformIndex];

		// Откройте документацию по "OpenCL Runtime" -> "Query Platform Info" -> "clGetPlatformInfo"
		// Не забывайте проверять коды ошибок с помощью макроса OCL_SAFE_CALL

		// TODO 1.1 +
		// Попробуйте вместо CL_PLATFORM_NAME передать какое-нибудь случайное число - например 239
		// Т.к. это некорректный идентификатор параметра платформы - то метод вернет код ошибки
		// Макрос OCL_SAFE_CALL заметит это, и кинет ошибку с кодом
		// Откройте таблицу с кодами ошибок:
		// libs/clew/CL/cl.h:103
		// P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с номером строки: cl.h:103) -> Enter
		// Найдите там нужный код ошибки и ее название
		// Затем откройте документацию по clGetPlatformInfo и в секции Errors найдите ошибку, с которой столкнулись
		// в документации подробно объясняется, какой ситуации соответствует данная ошибка, и это позволит, проверив код, понять, чем же вызвана данная ошибка (некорректным аргументом param_name)
		// Обратите внимание, что в этом же libs/clew/CL/cl.h файле указаны всевоможные defines, такие как CL_DEVICE_TYPE_GPU и т.п.

		// TODO 1.2 +
		// Аналогично тому, как был запрошен список идентификаторов всех платформ - так и с названием платформы, теперь, когда известна длина названия - его можно запросить:
		std::cout << "    Platform name: " << PlatformInfo(platform, CL_PLATFORM_NAME).data() << std::endl;

		// TODO 1.3 +
		// Запросите и напечатайте так же в консоль вендора данной платформы
		std::cout << "    Vendor name: " << PlatformInfo(platform, CL_PLATFORM_VENDOR).data() << std::endl;

		// TODO 2.1 + 
		// Запросите число доступных устройств данной платформы (аналогично тому, как это было сделано для запроса числа доступных платформ - см. секцию "OpenCL Runtime" -> "Query Devices")
		cl_uint devicesCount = 0;
		OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &devicesCount));

        std::vector<cl_device_id> devices(devicesCount);
		OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, devicesCount, devices.data(), nullptr));

		for(int deviceIndex = 0; deviceIndex < devicesCount; ++deviceIndex)
		{
			// TODO 2.2 +
			// Запросите и напечатайте в консоль:
			// - Название устройства
			// - Тип устройства (видеокарта/процессор/что-то странное)
			// - Размер памяти устройства в мегабайтах
			// - Еще пару или более свойств устройства, которые вам покажутся наиболее интересными

            cl_device_id device = devices[deviceIndex];

            // 1. Название устройства
			std::cout << "          Device name: " << DeviceInfo(device, CL_DEVICE_NAME).data() << std::endl;

            // 2. Тип устройства
			cl_device_type deviceType;
			clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, nullptr);
			std::string typeStr;
			if(deviceType & CL_DEVICE_TYPE_GPU)
				typeStr = "GPU";
			else if(deviceType & CL_DEVICE_TYPE_CPU)
				typeStr = "CPU";
			else
				typeStr = "Other";
			std::cout << "          Device type: " << typeStr << std::endl;

            // 3. Размер глобальной памяти в мегабайтах
            cl_ulong globalMemSize = 0;
			clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(globalMemSize), &globalMemSize, nullptr);
			std::cout << "          Global memory size: " << (globalMemSize / (1024 * 1024)) << " MB" << std::endl;

            // Максимальная тактовая частота (МГц)
			cl_uint maxClockFreq = 0;
			clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(maxClockFreq), &maxClockFreq, nullptr);
			std::cout << "          Max clock frequency: " << maxClockFreq << " MHz" << std::endl;

            // Версия OpenCL, поддерживаемая устройством
			std::cout << "          OpenCL version: " << DeviceInfo(device, CL_DEVICE_VERSION).data() << std::endl;
		}
	}

	return 0;
}
