
#include "encoder-nvidia-nvenc.hpp"
#include "obs/gs/gs-helper.hpp"

#include <memory>
#include <sstream>

//Instance
streamfx::encoder::nvidia::nvenc::h264_instance::h264_instance(obs_data_t* settings, obs_encoder_t* self, bool is_hw)
	: obs::encoder_instance(settings, self, is_hw)
{
	_nv_func = streamfx::encoder::nvidia::nvenc::h264_factory::get()->get_nv_func();

	//define parameters for encoder instance
	_nv_enc_params.version    = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
	_nv_enc_params.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;

	auto gctx             = streamfx::obs::gs::context();
	_nv_enc_params.device = gs_get_device_obj();

	//set up the encoder instance as _nv_enc_instance
	auto return_code = _nv_func.nvEncOpenEncodeSessionEx(&_nv_enc_params, &_nv_enc_instance);

	if (return_code != NV_ENC_SUCCESS) {
		std::stringstream str;
		str << "Could not create NVENC Session. Error Code: " << return_code;
		throw std::runtime_error(str.str());
	};

	//inizialize encoder instance with settings

	video_t*                        _obs_enc_video      = obs_encoder_video(_self);
	const struct video_output_info* _obs_enc_video_info = video_output_get_info(_obs_enc_video);

	//mandatory or crash
	_nv_enc_init_params.encodeGUID   = NV_ENC_CODEC_H264_GUID;
	_nv_enc_init_params.encodeHeight = _obs_enc_video_info->height;
	_nv_enc_init_params.encodeWidth  = _obs_enc_video_info->width;

	//optional
	_nv_enc_init_params.frameRateDen = _obs_enc_video_info->fps_den;
	_nv_enc_init_params.frameRateNum = _obs_enc_video_info->fps_num;

	_nv_enc_init_params.enableEncodeAsync = 1;

	return_code = _nv_func.nvEncInitializeEncoder(_nv_enc_instance, &_nv_enc_init_params);

	if (return_code != NV_ENC_SUCCESS) {
		std::stringstream str;
		str << "NVENC Session failed to initialize. Error Code: " << return_code;
		throw std::runtime_error(str.str());
	};

	//init buffer settings
	_nv_enc_init_input_buffer_params.version = NV_ENC_CREATE_INPUT_BUFFER_VER;
	_nv_enc_init_input_buffer_params.height  = _obs_enc_video_info->height;
	_nv_enc_init_input_buffer_params.width   = _obs_enc_video_info->width;

	video_format _format = _obs_enc_video_info->format;
	switch (_format) {
	case VIDEO_FORMAT_NV12:
		_nv_enc_init_input_buffer_params.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12;
		break;
	case VIDEO_FORMAT_I010:
		_nv_enc_init_input_buffer_params.bufferFmt = NV_ENC_BUFFER_FORMAT_YUV420_10BIT;
		break;
	case VIDEO_FORMAT_I444:
		_nv_enc_init_input_buffer_params.bufferFmt = NV_ENC_BUFFER_FORMAT_YUV444;
		break;
	case VIDEO_FORMAT_AYUV:
		_nv_enc_init_input_buffer_params.bufferFmt = NV_ENC_BUFFER_FORMAT_AYUV;
		break;
	default:
		std::stringstream str;
		str << "NVENC: Video Format not supported.";
		throw std::runtime_error(str.str());
	}

	//init bistream buffer settings
	_nv_enc_init_bitstream_buffer_params.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;

}

streamfx::encoder::nvidia::nvenc::h264_instance::~h264_instance()
{
	//destroy the encoder session
	_nv_func.nvEncDestroyEncoder(_nv_enc_instance);
	_nv_enc_instance = NULL;
}

void streamfx::encoder::nvidia::nvenc::h264_instance::migrate(obs_data_t* settings, uint64_t version) {}

bool streamfx::encoder::nvidia::nvenc::h264_instance::update(obs_data_t* settings)
{
	return false;
}

bool streamfx::encoder::nvidia::nvenc::h264_instance::encode_video(struct encoder_frame*  frame,
																   struct encoder_packet* packet, bool* received_packet)
{
	//software
	return false;
}

bool streamfx::encoder::nvidia::nvenc::h264_instance::encode_video(uint32_t handle, int64_t pts, uint64_t lock_key,
																   uint64_t* next_key, struct encoder_packet* packet,
																   bool* received_packet)
{
	//hardware


	return false;
}

bool streamfx::encoder::nvidia::nvenc::h264_instance::get_extra_data(uint8_t** extra_data, size_t* size)
{
	return false;
}

bool streamfx::encoder::nvidia::nvenc::h264_instance::get_sei_data(uint8_t** sei_data, size_t* size)
{
	return false;
}

void streamfx::encoder::nvidia::nvenc::h264_instance::get_video_info(struct video_scale_info* info) {}

//Factory
streamfx::encoder::nvidia::nvenc::h264_factory::h264_factory()
{
#if defined(D_PLATFORM_WINDOWS)
#if defined(D_PLATFORM_64BIT)
	std::filesystem::path lib_name = "nvEncodeAPI64.dll";
#else
	std::filesystem::path lib_name = "nvEncodeAPI.dll";
#endif
#else
	std::filesystem::path lib_name = "libnvidia-encode.so.1";
#endif

	_library = streamfx::util::library::load(lib_name);
	if (auto return_code = reinterpret_cast<decltype(&NvEncodeAPICreateInstance)>(
			_library->load_symbol("NvEncodeAPICreateInstance"))(&_nv_func);
		return_code != NV_ENC_SUCCESS) {
		std::stringstream str;
		str << "Could not load NVENC Functions. Error Code: " << return_code;
		throw std::runtime_error(str.str());
	};

	_info.id    = S_PREFIX "nvidia-nvenc-h264";
	_info.type  = obs_encoder_type::OBS_ENCODER_VIDEO;
	_info.codec = "h264";
	_info.caps  = OBS_ENCODER_CAP_DYN_BITRATE;

	finish_setup();
}

streamfx::encoder::nvidia::nvenc::h264_factory::~h264_factory() {}

const char* streamfx::encoder::nvidia::nvenc::h264_factory::get_name()
{
	return "NVIDIA NVENC H.264/AVC";
}

void streamfx::encoder::nvidia::nvenc::h264_factory::get_defaults2(obs_data_t* data) {}

void streamfx::encoder::nvidia::nvenc::h264_factory::migrate(obs_data_t* data, uint64_t version) {}

obs_properties_t* streamfx::encoder::nvidia::nvenc::h264_factory::get_properties2(instance_t* data)
{
	return nullptr;
}

std::shared_ptr<streamfx::encoder::nvidia::nvenc::h264_factory> _nvidia_nvenc_h264_factory_instance = nullptr;

NV_ENCODE_API_FUNCTION_LIST streamfx::encoder::nvidia::nvenc::h264_factory::get_nv_func()
{
	return _nv_func;
};

//Singleton
void streamfx::encoder::nvidia::nvenc::h264_factory::initialize()
{
	if (!_nvidia_nvenc_h264_factory_instance) {
		_nvidia_nvenc_h264_factory_instance = std::make_shared<h264_factory>();
	}
}

void streamfx::encoder::nvidia::nvenc::h264_factory::finalize()
{
	_nvidia_nvenc_h264_factory_instance.reset();
}

std::shared_ptr<streamfx::encoder::nvidia::nvenc::h264_factory> streamfx::encoder::nvidia::nvenc::h264_factory::get()
{
	return _nvidia_nvenc_h264_factory_instance;
}
