
#pragma once

#include "obs/obs-encoder-factory.hpp"
#include "nvEncodeAPI.h"

namespace streamfx::encoder::nvidia::nvenc {
	
	class h264_instance : public obs::encoder_instance {
		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS _nv_enc_params;
		NV_ENC_INITIALIZE_PARAMS             _nv_enc_init_params;
		NV_ENC_CREATE_INPUT_BUFFER           _nv_enc_init_input_buffer_params;
		NV_ENC_CREATE_BITSTREAM_BUFFER       _nv_enc_init_bitstream_buffer_params;
		void*                                _nv_enc_instance;

		NV_ENCODE_API_FUNCTION_LIST          _nv_func;

		public:
		h264_instance(obs_data_t* settings, obs_encoder_t* self, bool is_hw);
		virtual ~h264_instance();

		virtual void migrate(obs_data_t* settings, uint64_t version);

		virtual bool update(obs_data_t* settings);

		virtual bool encode_video(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet);
		virtual bool encode_video(uint32_t handle, int64_t pts, uint64_t lock_key, uint64_t* next_key,
								  struct encoder_packet* packet, bool* received_packet);

		virtual bool   get_extra_data(uint8_t** extra_data, size_t* size);
		virtual bool   get_sei_data(uint8_t** sei_data, size_t* size);
		virtual void   get_video_info(struct video_scale_info* info);
	};

	class h264_factory : public obs::encoder_factory<h264_factory, h264_instance> {
		std::shared_ptr<::streamfx::util::library> _library;
		NV_ENCODE_API_FUNCTION_LIST                _nv_func;

		public:
		h264_factory();
		virtual ~h264_factory();

		const char* get_name() override;

		void get_defaults2(obs_data_t* data) override;

		void migrate(obs_data_t* data, uint64_t version) override;

		obs_properties_t* get_properties2(instance_t* data) override;

		NV_ENCODE_API_FUNCTION_LIST get_nv_func();

		public: // Singleton
		static void initialize();

		static void finalize();

		static std::shared_ptr<h264_factory> get();

	};
} // namespace streamfx::encoder::nvidia::nvenc
