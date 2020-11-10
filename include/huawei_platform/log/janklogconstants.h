
#define JLID_POWERKEY_PRESS                                   0
#define JLID_USBCHARGING_START                                1
#define JLID_USBCHARGING_END                                  2
#define JLID_NEWRINGING_CONNECTION                            3
#define JLID_INCOMINGCALL_RINGING                             4
#define JLID_PMS_WAKEFULNESS_ASLEEP                           5
#define JLID_PMS_WAKEFULNESS_DREAMING                         6
#define JLID_PMS_WAKEFULNESS_NAPPING                          7
#define JLID_PMS_WAKEUP_FINISHED                              8
#define JLID_POWERKEY_RELEASE                                 9
#define JLID_KERNEL_LCD_OPEN                                  10
#define JLID_KERNEL_LCD_BACKLIGHT_OFF                         11
#define JLID_KERNEL_LCD_POWER_ON                              12
#define JLID_KERNEL_LCD_POWER_OFF                             13
#define JLID_KERNEL_PM_SUSPEND_WAKEUP                         14
#define JLID_GO_TO_SLEEP_REASON_USER                          15
#define JLID_KERNEL_PM_SUSPEND_SLEEP                          16
#define JLID_KERNEL_PM_DEEPSLEEP_WAKEUP                       17
#define JLID_KERNEL_LCD_BACKLIGHT_ON                          18
#define JLID_SYSPROC_INIT_POWERON_START                       19
#define JLID_SYSPROC_INIT_POWERON_END                         20
#define JLID_SERVICEMANAGER_POWERON_START                     21
#define JLID_SERVICEMANAGER_STARTUP                           22
#define JLID_START_SYSTEMSERVER                               23
#define JLID_LAUNCHER_STARTUP                                 24
#define JLID_ZYGOTE_START                                     25
#define JLID_FIRST_BOOT                                       26
#define JLID_BOOT_PROGRESS_START                              27
#define JLID_BOOT_PROGRESS_PRELOAD_START                      28
#define JLID_BOOT_PROGRESS_PRELOAD_END                        29
#define JLID_BOOT_PROGRESS_SYSTEM_RUN                         30
#define JLID_BOOT_PROGRESS_PMS_START                          31
#define JLID_BOOT_PROGRESS_PMS_READY                          32
#define JLID_BOOT_PROGRESS_AMS_READY                          33
#define JLID_BOOT_PROGRESS_ENABLE_SCREEN                      34
#define JLID_PROXIMITY_SENSOR_FAR                             35
#define JLID_PROXIMITY_SENSOR_NEAR                            36
#define JLID_JANK_FRAME_SKIP                                  37
#define JLID_JANK_FRAME_INFLATE_TIME                          38
#define JLID_JANK_FRAME_OBTAIN_TIME                           39
#define JLID_JANK_FRAME_SETUP_TIME                            40
#define JLID_JANK_FRAME_COMPOSE_TIME                          41
#define JLID_SLIDE_TO_CLICK                                   42
#define JLID_APP_LAUNCHING_BEGIN                              43
#define JLID_APP_LAUNCHING_END                                44
#define JLID_COVER_SENSOR_OPEN                                45
#define JLID_BOOT_PROGRESS_INITZYGOTE_START                   46
#define JLID_RIL_RESPONSE_NEW_SMS                             47
#define JLID_DISPATCH_NORMAL_SMS                              48
#define JLID_SEND_BROADCAST_SMS                               49
#define JLID_DISPATCH_SMS_FAILED                              50
#define JLID_WAP_DISPATCH_PDU                                 51
#define JLID_TP_GESTURE_KEY                                   52
#define JLID_WAKEUP_DBCLICK                                   53
#define JLID_INPUTDISPATCH_FINGERPRINT                        54
#define JLID_FINGER_INDENTIFY_OK                              55
#define JLID_FINGER_INDENTIFY_FAILED                          56
#define JLID_ROTATION_CHANGED                                 57
#define JLID_START_ROTATION_ANIM                              58
#define JLID_END_ROTATION_ANIM                                59
#define JLID_MONKEY_CTS_START                                 60
#define JLID_MONKEY_CTS_END                                   61
#define JLID_SQLITE_INSERT_ET                                 62
#define JLID_SQLITE_UPDATE_ET                                 63
#define JLID_SQLITE_QUERY_ET                                  64
#define JLID_SQLITE_DELETE_ET                                 65
#define JLID_SQLITE_EXECSQL_ET                                66
#define JLID_OLD_JANK_FRAME_SKIP                              67
#define JLID_JANK_FRAME_TIMEOUT                               68
#define JLID_SLIDE_TO_CLICK_TIMEOUT                           69
#define JLID_BINDER_TRANSACT_TIMEOUT                          70
#define JLID_KEYGUARD_DELEGATE_SCTURNON                       71
#define JLID_KEYGUARD_MEDIA_SCTURNON                          72
#define JLID_KEYGUARD_MEDIA_NTSCON                            73
#define JLID_KEYGUARD_MEDIA_HDLSCON                           74
#define JLID_KEYGUARD_MANAG_SCTURNON                          75
#define JLID_KEYGUARD_DELEGATE_SHOWN                          76
#define JLID_WAKEUP_REASON_PROX                               77
#define JLID_GO_TO_SLEEP_REASON_PROX                          78
#define JLID_GO_TO_SLEEP_REASON_OTHERS                        79
#define JLID_STRICT_MODE                                      80
#define JLID_JANK_FRAME_ENABLE                                81
#define JLID_JANK_FRAME_DISABLE                               82
#define JLID_COVER_OPEN                                       83
#define JLID_COVER_REMOVE_SCREEN_BEGIN                        84
#define JLID_COVER_REMOVE_SCREEN_END                          85
#define JLID_COVER_OPEN_BROADCAST                             86
#define JLID_CAMERA_LAUNCH_BEGIN                              87
#define JLID_CAMERA_LAUNCH_END                                88
#define JLID_CAMERA_AF_BEGIN                                  89
#define JLID_CAMERA_AF_END                                    90
#define JLID_CAMERA_SHUTTERLAG_ZSL_BEGIN                      91
#define JLID_CAMERA_SHUTTERLAG_ZSL_END                        92
#define JLID_CAMERA_SHUTTERLAG_NOZSL_BEGIN                    93
#define JLID_CAMERA_SHUTTERLAG_NOZSL_END                      94
#define JLID_CAMERA_SHOT_TO_SEE_BEGIN                         95
#define JLID_CAMERA_SHOT_TO_SEE_END                           96
#define JLID_CAMERA_SEE_TO_REVIEW_BEGIN                       97
#define JLID_CAMERA_SEE_TO_REVIEW_END                         98
#define JLID_CAMERA_QUICK_SNAPSHOT_BEGIN                      99
#define JLID_CAMERA_QUICK_SNAPSHOT_END                        100
#define JLID_CAMERA_START_RECORDING_BEGIN                     101
#define JLID_CAMERA_START_RECORDING_END                       102
#define JLID_ART_GC_TOTALTIME                                 103
#define JLID_ART_GC_PAUSETIME                                 104
#define JLID_KERNEL_HUNG_TASK                                 105
#define JLID_PMS_OPEN_ABC                                     106
#define JLID_ABC_OPEN_ALS                                     107
#define JLID_ABC_GET_FIRST_ALS_VALUE                          108
#define JLID_ABC_GET_SECOND_ALS_VALUE                         109
#define JLID_ABC_SET_BRIGHTNESS_BEGIN                         110
#define JLID_ABC_SET_BRIGHTNESS_END                           111
#define JLID_PEOPLE_ACTIVITY_ONCREATE                         112
#define JLID_CONTACT_DETAIL_LIST_BIND_VIEW                    113
#define JLID_CALLLOG_LIST_BIND_VIEW                           114
#define JLID_CONTACT_DETAIL_BIND_VIEW                         115
#define JLID_CONTACT_BIND_EDITOR_FOR_NEW                      116
#define JLID_DIALPAD_AFTER_TEXT_CHANGE                        117
#define JLID_CONTACT_MULTISELECT_ACTIVITY_ONCREATE            118
#define JLID_CONTACT_MULTISELECT_BIND_VIEW                    119
#define JLID_DIALPAD_ONTOUCH_NOT_FIRST_DOWN                   120
#define JLID_DIALPAD_ADAPTER_GET_VIEW                         121
#define JLID_DEF_CONTACT_ITEM_CLICK                           122
#define JLID_NEW_CONTACT_CLICK                                123
#define JLID_NEW_CONTACT_SAVE_CLICK                           124
#define JLID_EDIT_CONTACT_CLICK                               125
#define JLID_NEW_CONTACT_SELECT_ACCOUNT                       126
#define JLID_PEOPLE_ACTIVITY_ONNEWINTENT                      127
#define JLID_PHONE_INCALLUI_LAUNCH_START                      128
#define JLID_PHONE_INCALLUI_LAUNCH_END                        129
#define JLID_PHONE_INCALLUI_CLOSE_START                       130
#define JLID_PHONE_INCALLUI_CLOSE_END                         131
#define JLID_EDIT_CONTACT_END                                 132
#define JLID_SYSTEMUI_DROPDOWN_RESPONSE                       133
#define JLID_SYSTEMUI_TAB_SWITCH                              134
#define JLID_SYSTEMUI_START_RECENT                            135
#define JLID_SYSTEMUI_REMOVEALL_RECENT                        136
#define JLID_MMS_CONVERSATIONS_DELETE                         137
#define JLID_MMS_MESSAGES_DELETE                              138
#define JLID_MMS_MESSAGE_SEARCH                               139
#define JLID_MMS_RECENT_CONTACTS_QUERY                        140
#define JLID_MMS_MATCHED_CONTACTS_SEARCH                      141
#define JLID_APP_RUN_FRONT                                    142
#define JLID_RESOURCE_MANAGER                                 143
#define JLID_SETTINGS_BLUETOOTH_LAUNCH                        144
#define JLID_SETTINGS_WIFI_LAUNCH                             145
#define JLID_CAMERA_ONCREATE                                  146
#define JLID_CAMERA_ONRESUME                                  147
#define JLID_CAMERA_OPENCAMERA_BEGIN                          148
#define JLID_CAMERA_OPENCAMERA_END                            149
#define JLID_CAMERA_STARTPREVIEW_BEGIN                        150
#define JLID_CAMERA_STARTPREVIEW_END                          151
#define JLID_CAMERA_COLD_START_END                            152
#define JLID_CAMERA_HOT_START_END                             153
#define JLID_CAMERA_HOT_START_BEGIN                           154
#define JLID_CAMERA3_DEVICEPOWERUP_BEGIN                      155
#define JLID_CAMERA3_DEVICEPOWERUP_END                        156
#define JLID_CAMERA3_ISPPOWERUP_BEGIN                         157
#define JLID_CAMERA3_ISPPOWERUP_END                           158
#define JLID_CAMERA3_SENSORPOWERUP_BEGIN                      159
#define JLID_CAMERA3_SENSORPOWERUP_END                        160
#define JLID_CAMERA3_D2POWERUP_BEGIN                          161
#define JLID_CAMERA3_D2POWERUP_END                            162
#define JLID_CAMERA3_LASERPOWERUP_BEGIN                       163
#define JLID_CAMERA3_LASERPOWERUP_END                         164
#define JLID_CAMERA3_ACQUIRE_LASER_BEGIN                      165
#define JLID_CAMERA3_ACQUIRE_LASER_END                        166
#define JLID_CAMERA3_ACQUIRE_CAMERA_BEGIN                     167
#define JLID_CAMERA3_ACQUIRE_CAMERA_END                       168
#define JLID_CAMERA3_PIPELINE_START_BEGIN                     169
#define JLID_CAMERA3_PIPELINE_START_END                       170
#define JLID_CAMERA3_STARTSTREAMING_BEGIN                     171
#define JLID_CAMERA3_STARTSTREAMING_END                       172
#define JLID_EMUI_ANIMATION_JANKFRAME                         173
#define JLID_AUDIO_SOUNDEFFECT_BEGIN                          174
#define JLID_AUDIO_SOUNDEFFECT_END                            175
#define JLID_CAMERA3_RAW2YUV_BEGIN                            176
#define JLID_CAMERA3_RAW2YUV_END                              177
#define JLID_CAMERA3_PP_BEGIN                                 178
#define JLID_CAMERA3_PP_END                                   179
#define JLID_CAMERA3_JPEG_BEGIN                               180
#define JLID_CAMERA3_JPEG_END                                 181
#define JLID_CAMERA3_CAPTUREPROC_BEGIN                        182
#define JLID_CAMERA3_CAPTUREPROC_END                          183
#define JLID_CAMERA_FACING_FRONT                              184
#define JLID_CAMERA_FACING_REAR                               185
#define JLID_CAMERA_PREFLASH                                  186
#define JLID_CAMERA_FLASH                                     187
#define JLID_CAMERAALGO_ALLFOCUS_BEGIN                        188
#define JLID_CAMERAALGO_ALLFOCUS_END                          189
#define JLID_CAMERAALGO_BEAUTY_BEGIN                          190
#define JLID_CAMERAALGO_BEAUTY_END                            191
#define JLID_CAMERAALGO_BESTSHOT_BEGIN                        192
#define JLID_CAMERAALGO_BESTSHOT_END                          193
#define JLID_CAMERAALGO_BSHUTTER_BEGIN                        194
#define JLID_CAMERAALGO_BSHUTTER_END                          195
#define JLID_CAMERAALGO_CALIBINFO_BEGIN                       196
#define JLID_CAMERAALGO_CALIBINFO_END                         197
#define JLID_CAMERAALGO_COLOREFFECT_BEGIN                     198
#define JLID_CAMERAALGO_COLOREFFECT_END                       199
#define JLID_CAMERAALGO_DAVINCI_BEGIN                         200
#define JLID_CAMERAALGO_DAVINCI_END                           201
#define JLID_CAMERAALGO_DAVINCIDSP_BEGIN                      202
#define JLID_CAMERAALGO_DAVINCIDSP_END                        203
#define JLID_CAMERAALGO_DIGITALZOOM_BEGIN                     204
#define JLID_CAMERAALGO_DIGITALZOOM_END                       205
#define JLID_CAMERAALGO_DSFBEAUTY_BEGIN                       206
#define JLID_CAMERAALGO_DSFBEAUTY_END                         207
#define JLID_CAMERAALGO_DUALDEPTH_BEGIN                       208
#define JLID_CAMERAALGO_DUALDEPTH_END                         209
#define JLID_CAMERAALGO_EDOF_BEGIN                            210
#define JLID_CAMERAALGO_EDOF_END                              211
#define JLID_CAMERAALGO_FORMATCONVERT_BEGIN                   212
#define JLID_CAMERAALGO_FORMATCONVERT_END                     213
#define JLID_CAMERAALGO_HDFLASH_BEGIN                         214
#define JLID_CAMERAALGO_HDFLASH_END                           215
#define JLID_CAMERAALGO_HDRDSP_BEGIN                          216
#define JLID_CAMERAALGO_HDRDSP_END                            217
#define JLID_CAMERAALGO_LDC_BEGIN                             218
#define JLID_CAMERAALGO_LDC_END                               219
#define JLID_CAMERAALGO_MULTIDENOISE_BEGIN                    220
#define JLID_CAMERAALGO_MULTIDENOISE_END                      221
#define JLID_CAMERAALGO_MULTIDENOISEDSP_BEGIN                 222
#define JLID_CAMERAALGO_MULTIDENOISEDSP_END                   223
#define JLID_CAMERAALGO_NICEFOOD_BEGIN                        224
#define JLID_CAMERAALGO_NICEFOOD_END                          225
#define JLID_CAMERAALGO_REFOCUS_BEGIN                         226
#define JLID_CAMERAALGO_REFOCUS_END                           227
#define JLID_CAMERAALGO_SR_BEGIN                              228
#define JLID_CAMERAALGO_SR_END                                229
#define JLID_CAMERAALGO_CONVERTFROMSTRIDE_BEGIN               230
#define JLID_CAMERAALGO_CONVERTFROMSTRIDE_END                 231
#define JLID_CAMERAALGO_UVNR_BEGIN                            232
#define JLID_CAMERAALGO_UVNR_END                              233
#define JLID_CAMERAALGO_COLORFUSION_BEGIN                     234
#define JLID_CAMERAALGO_COLORFUSION_END                       235
#define JLID_CAMERAALGO_DCMMI_BEGIN                           236
#define JLID_CAMERAALGO_DCMMI_END                             237
#define JLID_CAMERAALGO_HDCCALIB_BEGIN                        238
#define JLID_CAMERAALGO_HDCCALIB_END                          239
#define JLID_CAMERAALGO_HDFLASHDSP_BEGIN                      240
#define JLID_CAMERAALGO_HDFLASHDSP_END                        241
#define JLID_CAMERAALGO_HDR_BEGIN                             242
#define JLID_CAMERAALGO_HDR_END                               243
#define JLID_CAMERAALGO_LASERFOV_BEGIN                        244
#define JLID_CAMERAALGO_LASERFOV_END                          245
#define JLID_CAMERAALGO_PANO2D_BEGIN                          246
#define JLID_CAMERAALGO_PANO2D_END                            247
#define JLID_CAMERAALGO_CONVERTTOSTRIDE_BEGIN                 248
#define JLID_CAMERAALGO_CONVERTTOSTRIDE_END                   249
#define JLID_CAMERAALGO_HWBEAUTY_BEGIN                        250
#define JLID_CAMERAALGO_HWBEAUTY_END                          251
#define JLID_SNAPSHOTMODE_SINGLE                              252
#define JLID_SNAPSHOTMODE_HDR                                 253
#define JLID_SNAPSHOTMODE_MULTIDENOISE                        254
#define JLID_SNAPSHOTMODE_EDOF                                255
#define JLID_SNAPSHOTMODE_DIGITALZOOM                         256
#define JLID_SNAPSHOTMODE_BEAUTY                              257
#define JLID_SNAPSHOTMODE_BURST                               258
#define JLID_SNAPSHOTMODE_FASTSHOT                            259
#define JLID_SNAPSHOTMODE_FASTEDOFSHOT                        260
#define JLID_SNAPSHOTMODE_ALLFOCUS                            261
#define JLID_SNAPSHOTMODE_NICEFOOD                            262
#define JLID_SNAPSHOTMODE_BSHUTTER_LONG_EXPO                  263
#define JLID_SNAPSHOTMODE_BSHUTTER_MULTI_EXPO                 264
#define JLID_SNAPSHOTMODE_HIGH_DYNAMIC                        265
#define JLID_CAMERAAPP_CAPTURE_BEGIN                          266
#define JLID_CAMERAAPP_CAPTURE_END                            267
#define JLID_PROC_START                                       268
#define JLID_PROC_EXIST                                       269
#define JLID_SVC_START                                        270
#define JLID_SVC_EXIST                                        271
#define JLID_ACTIVITY_LAUNCH                                  272
#define JLID_ACTIVITY_SWITCH                                  273
#define JLID_APP_ANR                                          274
#define JLID_APP_CRASH                                        275
#define JLID_SYS_LMK                                          276
#define JLID_UBM_END                                          277
#define JLID_SET_CALLLOG                                      278
#define JLID_SET_CONTACT_LIST                                 279
#define JLID_PHONE_INCALLUI_CLOSE_EXCLUDE                     280
#define JLID_CAMERA3_THUMBNAIL_BEGIN                          281
#define JLID_CAMERA3_THUMBNAIL_END                            282
#define JLID_CAMERA3_FIRST_CAPTURE_RAW_TIMESTAMP              283
#define JLID_CAMERA3_HAL_FIRST_VAILD_PREVIEW_FRAME            284
#define JLID_CAMERA3_HAL_STREAM_JPEG_FRAME_DONE               285
#define JLID_CAMERA3_HAL_STREAM_VIDEO_FIRST_VAILD_FRAME       286
#define JLID_CAMERA3_HAL_TAF_BEGIN                            287
#define JLID_CAMERA3_HAL_TAF_END                              288
#define JLID_CAMERA3_TAF_CMD                                  289
#define JLID_CAMERA3_HAL_FPS_LOW                              290
#define JLID_CAMERA3_HAL_FPS_HIGH                             291
#define JLID_CAMERAAPP_TAF_BEGIN                              292
#define JLID_CAMERAAPP_TAF_END                                293
#define JLID_CAMERAAPP_SWITCH_CAMERA_BEGIN                    294
#define JLID_CAMERAAPP_SWITCH_CAMERA_END                      295
#define JLID_CAMERAAPP_BURST_CAPTURE_BEGIN                    296
#define JLID_CAMERAAPP_BURST_CAPTURE_END                      297
#define JLID_CAMERAAPP_VIDEO_RECORDING_BEGIN                  298
#define JLID_CAMERAAPP_VIDEO_RECORDING_END                    299
#define JLID_CAMERA3_SNAPSHOTMODE_HDR                         300
#define JLID_CAMERA3_SNAPSHOTMODE_EDOF                        301
#define JLID_CAMERA3_ALLOC_BUFFER_BEGIN                       302
#define JLID_CAMERA3_ALLOC_BUFFER_END                         303
#define JLID_CAMERA3_DEALLOC_BUFFER_BEGIN                     304
#define JLID_CAMERA3_DEALLOC_BUFFER_END                       305
#define JLID_CAMERAAPP_SAVEPHOTO_BEGIN                        306
#define JLID_CAMERAAPP_SAVEPHOTO_END                          307
#define JLID_CAMERA3_PROFESSIONAL_MODE                        308
#define JLID_HWSYSMGR_REQ_PERMISSION                          309
#define JLID_LAUNCHER_KILLED                                  310
#define JLID_OPENGL_JANK_FRAME_SKIP                           311
#define JLID_MISC_EVENT_STAT                                  312
#define JLID_MANUAL_DUMP_LOG                                  313
#define JLID_CAMERAALGO_3X_BEGIN                              314
#define JLID_CAMERAALGO_3X_END                                315
#define JLID_CAMERAALGO_BackBinning_BEGIN                     316
#define JLID_CAMERAALGO_BackBinning_END                       317
#define JLID_CAMERAALGO_AlgoExt1_BEGIN                        318
#define JLID_CAMERAALGO_AlgoExt1_END                          319
#define JLID_CAMERAALGO_AlgoExt2_BEGIN                        320
#define JLID_CAMERAALGO_AlgoExt2_END                          321
#define JLID_CAMERAALGO_AlgoExt3_BEGIN                        322
#define JLID_CAMERAALGO_AlgoExt3_END                          323
#define JLID_CAMERAALGO_AlgoExt4_BEGIN                        324
#define JLID_CAMERAALGO_AlgoExt4_END                          325
#define JLID_CAMERA3_OIS_FW_DOWNLOAD_BEGIN                    326
#define JLID_CAMERA3_OIS_FW_DOWNLOAD_END                      327
#define JLID_CAMERA3_HAL_CAF_BEGIN                            328
#define JLID_CAMERA3_HAL_CAF_END                              329
#define JLID_CAMERAAPP_SURFACE_AVALIABLE                      330
#define JLID_CAMERAAPP_CREATESESSION_BEGIN                    331
#define JLID_CAMERAAPP_CREATESESSION_END                      332
#define JLID_CAMERA_CLOSECAMERA_BEGIN                         333
#define JLID_CAMERA_CLOSECAMERA_END                           334
#define JLID_ACTIVITY_LAUNCHING_BEGIN                         335
#define JLID_ACTIVITY_LAUNCHING_HANDLE                        336
#define JLID_ACTIVITY_LAUNCHING_END                           337
#define JLID_PHONE_INCALLUI_LAUNCH_EXCLUDE                    338
#define JLID_APP_WARMSTART_BEGIN                              339
#define JLID_APP_WARMSTART_END                                340
#define JLID_APP_COLDSTART_BEGIN                              341
#define JLID_APP_COLDSTART_END                                342