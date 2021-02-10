SRC := sm_pwm_01c.c auto_init_sm_pwm_01c.c

ifneq (,$(filter saul,$(USEMODULE)))
  SRC += sm_pwm_01c_saul.c
endif

include $(RIOTBASE)/Makefile.base
