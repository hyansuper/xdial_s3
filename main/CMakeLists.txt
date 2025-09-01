file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}
                            "main.c"
                            "board/*.c"
                            "lv_port/*.c"
                            "server/*.c"
                            "lv_observers/*.c"
                            "audio_manager/*.c"
                            "periodic_data_update/*.c"
                            "apps/apps.c"
                            "apps/assets/*.c"

                        #   apps 

                            "apps/ai_chat_app.c"
                            "apps/home_app.c"
                            "apps/clock_app.c"
                            "apps/settings_app.c"
                            "apps/player_app.c"
                            "apps/crypto_app.c"
                            "apps/example_app.c"
                        )





idf_component_register(
                        SRCS ${source_files}
                        PRIV_INCLUDE_DIRS "server" "board" "lv_port" "apps" "periodic_data_update" "lv_observers" "audio_manager"
                        EMBED_FILES "server/index.html" "audio_manager/ff-16b-1c-44100hz.mp3"
                    )



add_compile_definitions(APP_VER="${PROJECT_VER}")