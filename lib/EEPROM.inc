;;*****************************************************************************
;;*****************************************************************************
;;  FILENAME:   EEPROM.inc
;;  Version: 1.7, Updated on 2012/9/21 at 11:59:0
;;  Generated by PSoC Designer 5.3.2710
;;
;;  DESCRIPTION:
;;    Assembler include file for EEPROM User Module
;;-----------------------------------------------------------------------------
;;  Copyright (c) Cypress Semiconductor 2012. All Rights Reserved.
;;*****************************************************************************
;;*****************************************************************************

include "e2promlib.inc"

;-----------------------------------------------
;  EQUATES
;-----------------------------------------------
EEPROM_START_BLOCK:              equ      1feh                       ; Starting block of EEPROM EEPROM device
EEPROM_START_ADDR:               equ      1feh * 64                  ; absolute address of EEPROM EEPROM device
EEPROM_LENGTH:                   equ      40h                        ; length of EEPROM EEPROM device

E2_WR_ARG_STACK_FRAME_SIZE:      equ       7    ; EEPROM_bE2Write: SIZE of the argument list in stack frame
E2_RD_ARG_STACK_FRAME_SIZE:      equ       6    ; EEPROM_bE2Read:  SIZE of argument list in stack frame

;-------------------------------------
;  RETURN VALUES
;-------------------------------------
EEPROM_NOERROR:                  equ      0                          ; Successfull completion
EEPROM_FAILURE:                  equ     -1                          ; Error condition
EEPROM_STACKOVERFLOW:            equ     -2                          ; Error Stack Overflow


; end of EEPROM.inc
