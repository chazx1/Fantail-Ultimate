#pragma once
enum ClientFrameStage_t {
	FRAME_UNDEFINED = -1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END
};

namespace hooks {
	extern std::unique_ptr<vmt_hook> client_hook;
	extern std::unique_ptr<vmt_hook> clientmode_hook;
	extern std::unique_ptr<vmt_hook> panel_hook;
	extern std::unique_ptr<vmt_hook> renderview_hook;
	extern std::unique_ptr<vmt_hook> surface_hook;
	extern std::unique_ptr<vmt_hook> modelrender_hook;

	extern WNDPROC wndproc_original;
	extern HWND window;

	void initialize() noexcept;
	void shutdown() noexcept;

	using create_move_fn = bool(__thiscall*)(i_client_mode*, float, c_usercmd*);
	using frame_stage_notify_fn = void(__thiscall*)(i_base_client_dll*, int);
	using paint_traverse_fn = void(__thiscall*)(i_panel*, unsigned int, bool, bool);
	using scene_end_fn = void(__thiscall*)(void*);
	using override_view_fn = void*(__fastcall*)(i_client_mode*, void* _this, c_viewsetup* setup);
	using lock_cursor_fn = void(__thiscall*)(void*);
	using do_post_screen_effects_fn = int(__thiscall*)(void *, int);
	using present_fn = long(__stdcall*)(IDirect3DDevice9*, RECT*, RECT*, HWND, RGNDATA*);
	using reset_fn = long(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	using draw_model_execute_fn = void(__thiscall*)(iv_model_render*, IMatRenderContext*, const draw_model_state_t&, const model_render_info_t&, matrix_t*);
	using on_screen_size_changed_fn = void(__thiscall*)(i_surface*, int, int);
	using draw_set_color_fn = void(__thiscall*)(i_surface*, int, int, int, int);

	int __stdcall do_post_screen_effects(int value) noexcept;
	bool __stdcall create_move(float frame_time, c_usercmd* user_cmd) noexcept;
	void __stdcall frame_stage_notify(int frame_stage) noexcept;
	void __stdcall paint_traverse(unsigned int panel, bool force_repaint, bool allow_force) noexcept;
	void __stdcall scene_end() noexcept;
	void __fastcall override_view(void* _this, void* _edx, c_viewsetup* setup) noexcept;
	void __stdcall lock_cursor() noexcept;
	LRESULT __stdcall wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept;
	long __stdcall present(IDirect3DDevice9* device, RECT* source_rect, RECT* dest_rect, HWND dest_window_override, RGNDATA* dirty_region) noexcept;
	long __stdcall reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* present_parameters) noexcept;
	void __stdcall draw_model_execute(IMatRenderContext * ctx, const draw_model_state_t & state, const model_render_info_t & info, matrix_t * bone_to_world)noexcept;
	float __stdcall viewmodel_fov()noexcept;
	void __stdcall draw_set_color(int r, int g, int b, int a) noexcept;
	void __stdcall on_screen_size_changed(int old_width, int old_height) noexcept;
}