define log_list
$(foreach v, $(1), $(info $(v)));
$(info log_list end: $(words $(1)) +++++++++++++++++++++++++++++++++++++++++++++++);
endef

all_sub_files_short= \
$(wildcard $(1)$(2)) \
$(wildcard $(1)**/$(2)) \
$(wildcard $(1)**/**/$(2)) \
$(wildcard $(1)**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/$(2))
all_sub_files_long= \
$(wildcard $(1)$(2)) \
$(wildcard $(1)**/$(2)) \
$(wildcard $(1)**/**/$(2)) \
$(wildcard $(1)**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/**/**/**/**/**/$(2)) \
$(wildcard $(1)**/**/**/**/**/**/**/**/**/**/**/$(2))

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

$(info start ------------------------------------------------------------)

root_folders := D:/ndk/android-r10e


#sub_folders = $(addsuffix /*,$(root_folders))
#sub_folders2 := $(wildcard $(sub_folders))
#$(call log_list, $(sub_folders2))

ALL_CPP := $(call rwildcard, $(root_folders)/,*.cpp)
$(call log_list, $(ALL_CPP))
ALL_CPP :=  $(call all_sub_files_long, $(root_folders),*.cpp)
$(call log_list, $(ALL_CPP))



$(info end --------------------------------------------------------------)
