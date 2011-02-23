/*
 * Copyright (C) 2011 The Paparazzi Team
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
* @file state.c
*   @brief General inteface for the main vehicle states.
*
*   This is the more detailed description of this file.
*
*   @author Felix Ruess <felix.ruess@gmail.com>
*
*/

#include "state.h"

struct State state;



/*******************************************************************************
 *                                                                             *
 * transformation functions for the POSITION representations                   *
 *                                                                             *
 ******************************************************************************/
/** @addtogroup PosGroup
 *  @{ */

void stateCalcPositionEcef_i(void) {
  if (bit_is_set(state.pos_status, POS_ECEF_I))
    return;

  if (bit_is_set(state.pos_status, POS_ECEF_F)) {
    ECEF_BFP_OF_REAL(state.ecef_pos_i, state.ecef_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_NED_I)) {
    //TODO check if resolution is good enough
    ecef_of_ned_point_i(&state.ecef_pos_i, &state.ned_origin_i, &state.ned_pos_i);
  }
  else if (bit_is_set(state.pos_status, POS_NED_F)) {
    /* transform ned_f to ecef_f, set status bit, then convert to int */
    ecef_of_ned_point_f(&state.ecef_pos_f, &state.ned_origin_f, &state.ned_pos_f);
    SetBit(state.pos_status, POS_ECEF_F);
    ECEF_BFP_OF_REAL(state.ecef_pos_i, state.ecef_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_LLA_F)) {
    /* transform lla_f to ecef_f, set status bit, then convert to int */
    ecef_of_lla_f(&state.ecef_pos_f, &state.lla_pos_f);
    SetBit(state.pos_status, POS_ECEF_F);
    ECEF_BFP_OF_REAL(state.ecef_pos_i, state.ecef_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_LLA_I)) {
    ecef_of_lla_i(&state.ecef_pos_i, &state.lla_pos_i);
  }
  else {
    /* could not get this representation,  set errno */
    //struct EcefCoor_i _ecef_zero = {0};
    //return _ecef_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.pos_status, POS_ECEF_I);

  //return state.ecef_pos_i;
}

void stateCalcPositionNed_i(void) {
  if (bit_is_set(state.pos_status, POS_NED_I))
    return;

  int errno = 0;
  if (state.ned_initialised_i) {
    if (bit_is_set(state.pos_status, POS_NED_F)) {
      NED_BFP_OF_REAL(state.ned_pos_i, state.ned_pos_f);
    }
    else if (bit_is_set(state.pos_status, POS_ECEF_I)) {
      ned_of_ecef_point_i(&state.ned_pos_i, &state.ned_origin_i, &state.ecef_pos_i);
    }
    else if (bit_is_set(state.pos_status, POS_ECEF_F)) {
      /* transform ecef_f -> ned_f, set status bit, then convert to int */
      ned_of_ecef_point_f(&state.ned_pos_f, &state.ned_origin_f, &state.ecef_pos_f);
      SetBit(state.pos_status, POS_NED_F);
      NED_BFP_OF_REAL(state.ned_pos_i, state.ned_pos_f);
    }
    else if (bit_is_set(state.pos_status, POS_LLA_F)) {
      /* transform lla_f -> ecef_f -> ned_f, set status bits, then convert to int */
      ecef_of_lla_f(&state.ecef_pos_f, &state.lla_pos_f);
      SetBit(state.pos_status, POS_ECEF_F);
      ned_of_ecef_point_f(&state.ned_pos_f, &state.ned_origin_f, &state.ecef_pos_f);
      SetBit(state.pos_status, POS_NED_F);
      NED_BFP_OF_REAL(state.ned_pos_i, state.ned_pos_f);
    }
    else if (bit_is_set(state.pos_status, POS_LLA_I)) {
      ned_of_lla_point_i(&state.ned_pos_i, &state.ned_origin_i, &state.lla_pos_i);
    }
    else { /* could not get this representation,  set errno */
      errno = 1;
    }
  } else { /* ned coordinate system not initialized,  set errno */
    errno = 2;
  }
  if (errno) {
    //struct NedCoor_i _ned_zero = {0};
    //return _ned_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.pos_status, POS_NED_I);

  //return state.ned_pos_i;
}

void stateCalcPositionLla_i(void) {
  if (bit_is_set(state.pos_status, POS_LLA_I))
    return;

  if (bit_is_set(state.pos_status, POS_LLA_F)) {
    LLA_BFP_OF_REAL(state.lla_pos_i, state.lla_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_ECEF_I)) {
    lla_of_ecef_i(&state.lla_pos_i, &state.ecef_pos_i);
  }
  else if (bit_is_set(state.pos_status, POS_ECEF_F)) {
    /* transform ecef_f -> lla_f, set status bit, then convert to int */
    lla_of_ecef_f(&state.lla_pos_f, &state.ecef_pos_f);
    SetBit(state.pos_status, POS_LLA_F);
    LLA_BFP_OF_REAL(state.lla_pos_i, state.lla_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_NED_F)) {
    /* transform ned_f -> ecef_f -> lla_f -> lla_i, set status bits */
    ecef_of_ned_point_f(&state.ecef_pos_f, &state.ned_origin_f, &state.ned_pos_f);
    SetBit(state.pos_status, POS_ECEF_F);
    lla_of_ecef_f(&state.lla_pos_f, &state.ecef_pos_f);
    SetBit(state.pos_status, POS_LLA_F);
    LLA_BFP_OF_REAL(state.lla_pos_i, state.lla_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_NED_I)) {
    /* transform ned_i -> ecef_i -> lla_i, set status bits */
    //TODO check if resolution is enough
    ecef_of_ned_point_i(&state.ecef_pos_i, &state.ned_origin_i, &state.ned_pos_i);
    SetBit(state.pos_status, POS_ECEF_I);
    lla_of_ecef_i(&state.lla_pos_i, &state.ecef_pos_i); /* uses double version internally */
  }
  else {
    /* could not get this representation,  set errno */
    //struct LlaCoor_i _lla_zero = {0};
    //return _lla_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.pos_status, POS_LLA_I);

  //return state.lla_pos_i;
}

void stateCalcPositionEcef_f(void) {
  if (bit_is_set(state.pos_status, POS_ECEF_F))
    return;

  if (bit_is_set(state.pos_status, POS_ECEF_I)) {
    ECEF_FLOAT_OF_BFP(state.ecef_pos_f, state.ecef_pos_i);
  }
  else if (bit_is_set(state.pos_status, POS_NED_F)) {
    ecef_of_ned_point_f(&state.ecef_pos_f, &state.ned_origin_f, &state.ned_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_NED_I)) {
    /* transform ned_i -> ecef_i -> ecef_f, set status bits */
    //TODO check if resolution is good enough
    ecef_of_ned_point_i(&state.ecef_pos_i, &state.ned_origin_i, &state.ned_pos_i);
    SetBit(state.pos_status, POS_ECEF_F);
    ECEF_FLOAT_OF_BFP(state.ecef_pos_f, state.ecef_pos_i);
  }
  else if (bit_is_set(state.pos_status, POS_LLA_F)) {
    ecef_of_lla_f(&state.ecef_pos_f, &state.lla_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_LLA_I)) {
    LLA_FLOAT_OF_BFP(state.lla_pos_f, state.lla_pos_i);
    SetBit(state.pos_status, POS_LLA_F);
    ecef_of_lla_f(&state.ecef_pos_f, &state.lla_pos_f);
  }
  else {
    /* could not get this representation,  set errno */
    //struct EcefCoor_f _ecef_zero = {0.0f};
    //return _ecef_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.pos_status, POS_ECEF_F);

  //return state.ecef_pos_f;
}

void stateCalcPositionNed_f(void) {
  if (bit_is_set(state.pos_status, POS_NED_F))
    return;

  int errno = 0;
  if (state.ned_initialised_f) {
    if (bit_is_set(state.pos_status, POS_NED_I)) {
      NED_FLOAT_OF_BFP(state.ned_pos_f, state.ned_pos_i);
    }
    else if (bit_is_set(state.pos_status, POS_ECEF_F)) {
      ned_of_ecef_point_f(&state.ned_pos_f, &state.ned_origin_f, &state.ecef_pos_f);
    }
    else if (bit_is_set(state.pos_status, POS_ECEF_I)) {
      /* transform ecef_i -> ned_i -> ned_f, set status bits */
      ned_of_ecef_point_i(&state.ned_pos_i, &state.ned_origin_i, &state.ecef_pos_i);
      SetBit(state.pos_status, POS_NED_I);
      NED_FLOAT_OF_BFP(state.ned_pos_f, state.ned_pos_i);
    }
    else if (bit_is_set(state.pos_status, POS_LLA_F)) {
      ned_of_lla_point_f(&state.ned_pos_f, &state.ned_origin_f, &state.lla_pos_f);
    }
    else if (bit_is_set(state.pos_status, POS_LLA_I)) {
      /* transform lla_i -> ecef_i -> ned_i -> ned_f, set status bits */
      ecef_of_lla_i(&state.ecef_pos_i, &state.lla_pos_i); /* converts to doubles internally */
      SetBit(state.pos_status, POS_ECEF_I);
      ned_of_ecef_point_i(&state.ned_pos_i, &state.ned_origin_i, &state.ecef_pos_i);
      SetBit(state.pos_status, POS_NED_I);
      NED_FLOAT_OF_BFP(state.ned_pos_f, state.ned_pos_i);
    }
    else { /* could not get this representation,  set errno */
      errno = 1;
    }
  } else { /* ned coordinate system not initialized,  set errno */
    errno = 2;
  }
  if (errno) {
    //struct NedCoor_f _ned_zero = {0.0f};
    //return _ned_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.pos_status, POS_NED_F);

  //return state.ned_pos_f;
}

void stateCalcPositionLla_f(void) {
  if (bit_is_set(state.pos_status, POS_LLA_F))
    return;

  if (bit_is_set(state.pos_status, POS_LLA_I)) {
    LLA_FLOAT_OF_BFP(state.lla_pos_f, state.lla_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_ECEF_F)) {
    lla_of_ecef_f(&state.lla_pos_f, &state.ecef_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_ECEF_I)) {
    /* transform ecef_i -> ecef_f -> lla_f, set status bits */
    ECEF_FLOAT_OF_BFP(state.ecef_pos_f, state.ecef_pos_i);
    SetBit(state.pos_status, POS_ECEF_F);
    lla_of_ecef_f(&state.lla_pos_f, &state.ecef_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_NED_F)) {
    /* transform ned_f -> ecef_f -> lla_f, set status bits */
    ecef_of_ned_point_f(&state.ecef_pos_f, &state.ned_origin_f, &state.ned_pos_f);
    SetBit(state.pos_status, POS_ECEF_F);
    lla_of_ecef_f(&state.lla_pos_f, &state.ecef_pos_f);
  }
  else if (bit_is_set(state.pos_status, POS_NED_I)) {
    /* transform ned_i -> ned_f -> ecef_f -> lla_f, set status bits */
    NED_FLOAT_OF_BFP(state.ned_pos_f, state.ned_pos_i);
    SetBit(state.pos_status, POS_NED_F);
    ecef_of_ned_point_f(&state.ecef_pos_f, &state.ned_origin_f, &state.ned_pos_f);
    SetBit(state.pos_status, POS_ECEF_F);
    lla_of_ecef_f(&state.lla_pos_f, &state.ecef_pos_f);
  }
  else {
    /* could not get this representation,  set errno */
    //struct LlaCoor_f _lla_zero = {0.0};
    //return _lla_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.pos_status, POS_LLA_F);

  //return state.lla_pos_f;
}
/** @}*/





/******************************************************************************
 *                                                                            *
 * Transformation functions for the SPEED representations                     *
 *                                                                            *
 *****************************************************************************/
/** @addtogroup SpeedGroup
 *  @{ */
/************************ Set functions ****************************/

void stateCalcSpeedNed_i(void) {
  if (bit_is_set(state.speed_status, SPEED_NED_I))
    return;

  int errno = 0;
  if (state.ned_initialised_i) {
    if (bit_is_set(state.speed_status, SPEED_NED_F)) {
      SPEEDS_BFP_OF_REAL(state.ned_speed_i, state.ned_speed_f);
    }
    else if (bit_is_set(state.speed_status, SPEED_ECEF_I)) {
      ned_of_ecef_vect_i(&state.ned_speed_i, &state.ned_origin_i, &state.ecef_speed_i);
    }
    else if (bit_is_set(state.speed_status, SPEED_ECEF_F)) {
      /* transform ecef_f -> ecef_i -> ned_i , set status bits */
      SPEEDS_BFP_OF_REAL(state.ecef_speed_i, state.ecef_speed_f);
      SetBit(state.speed_status, SPEED_ECEF_I);
      ned_of_ecef_vect_i(&state.ned_speed_i, &state.ned_origin_i, &state.ecef_speed_i);
    }
    else { /* could not get this representation,  set errno */
      errno = 1;
    }
  } else { /* ned coordinate system not initialized,  set errno */
    errno = 2;
  }
  if (errno) {
    //struct NedCoor_i _ned_zero = {0};
    //return _ned_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_NED_I);

  //return state.ned_speed_i;
}

void stateCalcSpeedEcef_i(void) {
  if (bit_is_set(state.speed_status, SPEED_ECEF_I))
    return;

  if (bit_is_set(state.speed_status, SPEED_ECEF_F)) {
    SPEEDS_BFP_OF_REAL(state.ecef_speed_i, state.ecef_speed_f);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_I)) {
    ecef_of_ned_vect_i(&state.ecef_speed_i, &state.ned_origin_i, &state.ned_speed_i);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_F)) {
    /* transform ned_f -> ned_i -> ecef_i , set status bits */
    SPEEDS_BFP_OF_REAL(state.ned_speed_i, state.ned_speed_f);
    SetBit(state.speed_status, SPEED_NED_I);
    ecef_of_ned_vect_i(&state.ecef_speed_i, &state.ned_origin_i, &state.ned_speed_i);
  }
  else {
    /* could not get this representation,  set errno */
    //struct EcefCoor_i _ecef_zero = {0};
    //return _ecef_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_ECEF_I);

  //return state.ecef_speed_i;
}

void stateCalcHorizontalSpeedNorm_i(void) { //TODO
  if (bit_is_set(state.speed_status, SPEED_HNORM_I))
    return;

  if (bit_is_set(state.speed_status, SPEED_HNORM_F)){
    state.h_speed_norm_i = SPEED_BFP_OF_REAL(state.h_speed_norm_f);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_I)) {
    //TODO consider INT32_SPEED_FRAC
    //INT32_VECT2_NORM(state.h_speed_norm_i, state.ned_speed_i);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_F)) {
    FLOAT_VECT2_NORM(state.h_speed_norm_f, state.ned_speed_f);
    SetBit(state.speed_status, SPEED_HNORM_F);
    state.h_speed_norm_i = SPEED_BFP_OF_REAL(state.h_speed_norm_f);
  }
  else if (bit_is_set(state.speed_status, SPEED_ECEF_I)) {
    /* transform ecef speed to ned, set status bit, then compute norm */
    //foo
    //TODO consider INT32_SPEED_FRAC
    //INT32_VECT2_NORM(state.h_speed_norm_i, state.ned_speed_i);
  }
  else if (bit_is_set(state.speed_status, SPEED_ECEF_F)) {
    ned_of_ecef_vect_f(&state.ned_speed_f, &state.ned_origin_f, &state.ecef_speed_f);
    SetBit(state.speed_status, SPEED_NED_F);
    FLOAT_VECT2_NORM(state.h_speed_norm_f, state.ned_speed_f);
    SetBit(state.speed_status, SPEED_HNORM_F);
    state.h_speed_norm_i = SPEED_BFP_OF_REAL(state.h_speed_norm_f);
  }
  else {
    //int32_t _norm_zero = 0;
    //return _norm_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_HNORM_I);

  //return state.h_speed_norm_i;
}

void stateCalcHorizontalSpeedDir_i(void) { //TODO
  if (bit_is_set(state.speed_status, SPEED_HDIR_I))
    return;

  if (bit_is_set(state.speed_status, SPEED_HDIR_F)){
    state.h_speed_dir_i = SPEED_BFP_OF_REAL(state.h_speed_dir_f);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_I)) {
    //foo
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_HDIR_I);

  //return state.h_speed_dir_i;
}

void stateCalcSpeedNed_f(void) {
  if (bit_is_set(state.speed_status, SPEED_NED_F))
    return;

  int errno = 0;
  if (state.ned_initialised_f) {
    if (bit_is_set(state.speed_status, SPEED_NED_I)) {
      SPEEDS_FLOAT_OF_BFP(state.ned_speed_f, state.ned_speed_i);
    }
    else if (bit_is_set(state.speed_status, SPEED_ECEF_F)) {
      ned_of_ecef_vect_f(&state.ned_speed_f, &state.ned_origin_f, &state.ecef_speed_f);
    }
    else if (bit_is_set(state.speed_status, SPEED_ECEF_I)) {
      /* transform ecef_i -> ecef_f -> ned_f , set status bits */
      SPEEDS_FLOAT_OF_BFP(state.ecef_speed_f, state.ecef_speed_i);
      SetBit(state.speed_status, SPEED_ECEF_F);
      ned_of_ecef_vect_f(&state.ned_speed_f, &state.ned_origin_f, &state.ecef_speed_f);
    }
    else { /* could not get this representation,  set errno */
      errno = 1;
    }
  } else { /* ned coordinate system not initialized,  set errno */
    errno = 2;
  }
  if (errno) {
    //struct NedCoor_f _ned_zero = {0.0f};
    //return _ned_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_NED_F);

  //return state.ned_speed_f;
}

void stateCalcSpeedEcef_f(void) {
  if (bit_is_set(state.speed_status, SPEED_ECEF_F))
    return;

  if (bit_is_set(state.speed_status, SPEED_ECEF_I)) {
    SPEEDS_FLOAT_OF_BFP(state.ecef_speed_f, state.ned_speed_i);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_F)) {
    ecef_of_ned_vect_f(&state.ecef_speed_f, &state.ned_origin_f, &state.ned_speed_f);
  }
  else if (bit_is_set(state.speed_status, SPEED_NED_I)) {
    /* transform ned_f -> ned_i -> ecef_i , set status bits */
    SPEEDS_FLOAT_OF_BFP(state.ned_speed_f, state.ned_speed_i);
    SetBit(state.speed_status, SPEED_NED_F);
    ecef_of_ned_vect_f(&state.ecef_speed_f, &state.ned_origin_f, &state.ned_speed_f);
  }
  else {
    /* could not get this representation,  set errno */
    //struct EcefCoor_f _ecef_zero = {0.0f};
    //return _ecef_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_ECEF_F);

  //return state.ecef_speed_f;
}

void stateCalcHorizontalSpeedNorm_f(void) { //TODO
  if (bit_is_set(state.speed_status, SPEED_HNORM_F))
    return;

  if (bit_is_set(state.speed_status, SPEED_HNORM_I)){
    state.h_speed_norm_f = SPEED_FLOAT_OF_BFP(state.h_speed_norm_i);
  } else if (bit_is_set(state.speed_status, SPEED_NED_F)) {
    FLOAT_VECT2_NORM(state.h_speed_norm_f, state.ned_speed_f);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_HNORM_F);

  //return state.h_speed_norm_f;
}

void stateCalcHorizontalSpeedDir_f(void) { //TODO
  if (bit_is_set(state.speed_status, SPEED_HDIR_F))
    return;

  if (bit_is_set(state.speed_status, SPEED_HDIR_I)){
    state.h_speed_dir_f = SPEED_FLOAT_OF_BFP(state.h_speed_dir_i);
  } else if (bit_is_set(state.speed_status, SPEED_NED_F)) {
    //foo
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.speed_status, SPEED_HDIR_F);

  //return state.h_speed_dir_f;
}
/** @}*/



/******************************************************************************
 *                                                                            *
 * Transformation functions for the ACCELERATION representations              *
 *                                                                            *
 *****************************************************************************/
/** @addtogroup AccelGroup
 *  @{ */

void stateCalcAccelNed_i(void) {
  if (bit_is_set(state.accel_status, ACCEL_NED_I))
    return;

  int errno = 0;
  if (state.ned_initialised_i) {
    if (bit_is_set(state.accel_status, ACCEL_NED_F)) {
      ACCELS_BFP_OF_REAL(state.ned_accel_i, state.ned_accel_f);
    }
    else if (bit_is_set(state.accel_status, ACCEL_ECEF_I)) {
      ned_of_ecef_vect_i(&state.ned_accel_i, &state.ned_origin_i, &state.ecef_accel_i);
    }
    else if (bit_is_set(state.accel_status, ACCEL_ECEF_F)) {
      /* transform ecef_f -> ecef_i -> ned_i , set status bits */
      ACCELS_BFP_OF_REAL(state.ecef_accel_i, state.ecef_accel_f);
      SetBit(state.accel_status, ACCEL_ECEF_I);
      ned_of_ecef_vect_i(&state.ned_accel_i, &state.ned_origin_i, &state.ecef_accel_i);
    }
    else { /* could not get this representation,  set errno */
      errno = 1;
    }
  } else { /* ned coordinate system not initialized,  set errno */
    errno = 2;
  }
  if (errno) {
    //struct NedCoor_i _ned_zero = {0};
    //return _ned_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.accel_status, ACCEL_NED_I);

  //return state.ned_accel_i;
}

void stateCalcAccelEcef_i(void) {
  if (bit_is_set(state.accel_status, ACCEL_ECEF_I))
    return;

  if (bit_is_set(state.accel_status, ACCEL_ECEF_F)) {
    ACCELS_BFP_OF_REAL(state.ecef_accel_i, state.ecef_accel_f);
  }
  else if (bit_is_set(state.accel_status, ACCEL_NED_I)) {
    ecef_of_ned_vect_i(&state.ecef_accel_i, &state.ned_origin_i, &state.ned_accel_i);
  }
  else if (bit_is_set(state.accel_status, ACCEL_NED_F)) {
    /* transform ned_f -> ned_i -> ecef_i , set status bits */
    ACCELS_BFP_OF_REAL(state.ned_accel_i, state.ned_accel_f);
    SetBit(state.accel_status, ACCEL_NED_I);
    ecef_of_ned_vect_i(&state.ecef_accel_i, &state.ned_origin_i, &state.ned_accel_i);
  }
  else {
    /* could not get this representation,  set errno */
    //struct EcefCoor_i _ecef_zero = {0};
    //return _ecef_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.accel_status, ACCEL_ECEF_I);

  //return state.ecef_accel_i;
}

void stateCalcAccelNed_f(void) {
  if (bit_is_set(state.accel_status, ACCEL_NED_F))
    return;

  int errno = 0;
  if (state.ned_initialised_f) {
    if (bit_is_set(state.accel_status, ACCEL_NED_I)) {
      ACCELS_FLOAT_OF_BFP(state.ned_accel_f, state.ned_accel_i);
    }
    else if (bit_is_set(state.accel_status, ACCEL_ECEF_F)) {
      ned_of_ecef_vect_f(&state.ned_accel_f, &state.ned_origin_f, &state.ecef_accel_f);
    }
    else if (bit_is_set(state.accel_status, ACCEL_ECEF_I)) {
      /* transform ecef_i -> ecef_f -> ned_f , set status bits */
      ACCELS_FLOAT_OF_BFP(state.ecef_accel_f, state.ecef_accel_i);
      SetBit(state.accel_status, ACCEL_ECEF_F);
      ned_of_ecef_vect_f(&state.ned_accel_f, &state.ned_origin_f, &state.ecef_accel_f);
    }
    else { /* could not get this representation,  set errno */
      errno = 1;
    }
  } else { /* ned coordinate system not initialized,  set errno */
    errno = 2;
  }
  if (errno) {
    //struct NedCoor_f _ned_zero = {0.0f};
    //return _ned_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.accel_status, ACCEL_NED_F);

  //return state.ned_accel_f;
}

void stateCalcAccelEcef_f(void) {
  if (bit_is_set(state.accel_status, ACCEL_ECEF_F))
    return;

  if (bit_is_set(state.accel_status, ACCEL_ECEF_I)) {
    ACCELS_FLOAT_OF_BFP(state.ecef_accel_f, state.ned_accel_i);
  }
  else if (bit_is_set(state.accel_status, ACCEL_NED_F)) {
    ecef_of_ned_vect_f(&state.ecef_accel_f, &state.ned_origin_f, &state.ned_accel_f);
  }
  else if (bit_is_set(state.accel_status, ACCEL_NED_I)) {
    /* transform ned_f -> ned_i -> ecef_i , set status bits */
    ACCELS_FLOAT_OF_BFP(state.ned_accel_f, state.ned_accel_i);
    SetBit(state.accel_status, ACCEL_NED_F);
    ecef_of_ned_vect_f(&state.ecef_accel_f, &state.ned_origin_f, &state.ned_accel_f);
  }
  else {
    /* could not get this representation,  set errno */
    //struct EcefCoor_f _ecef_zero = {0.0f};
    //return _ecef_zero;
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.accel_status, ACCEL_ECEF_F);

  //return state.ecef_accel_f;
}
/** @}*/



/******************************************************************************
 *                                                                            *
 * Transformation functions for the ATTITUDE representations                  *
 *                                                                            *
 *****************************************************************************/
/** @addtogroup AttGroup
 *  @{ */

void stateCalcNedToBodyQuat_i(void) {
  if (bit_is_set(state.att_status, ATT_QUAT_I))
    return;

  if (bit_is_set(state.att_status, ATT_QUAT_F)) {
    QUAT_BFP_OF_REAL(state.ned_to_body_quat_i, state.ned_to_body_quat_f);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_I)) {
    INT32_QUAT_OF_RMAT(state.ned_to_body_quat_i, state.ned_to_body_rmat_i);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_I)) {
    INT32_QUAT_OF_EULERS(state.ned_to_body_quat_i, state.ned_to_body_eulers_i);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_F)) {
    RMAT_BFP_OF_REAL(state.ned_to_body_rmat_i, state.ned_to_body_rmat_f);
    SetBit(state.att_status, ATT_RMAT_I);
    INT32_QUAT_OF_RMAT(state.ned_to_body_quat_i, state.ned_to_body_rmat_i);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_F)) {
    EULERS_BFP_OF_REAL(state.ned_to_body_eulers_i, state.ned_to_body_eulers_f);
    SetBit(state.att_status, ATT_EULER_I);
    INT32_QUAT_OF_EULERS(state.ned_to_body_quat_i, state.ned_to_body_eulers_i);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.att_status, ATT_QUAT_I);

  //return state.ned_to_body_quat_i;
}

void stateCalcNedToBodyRMat_i(void) {
  if (bit_is_set(state.att_status, ATT_RMAT_I))
    return;

  if (bit_is_set(state.att_status, ATT_RMAT_F)) {
    RMAT_BFP_OF_REAL(state.ned_to_body_rmat_i, state.ned_to_body_rmat_f);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_I)) {
    INT32_RMAT_OF_QUAT(state.ned_to_body_rmat_i, state.ned_to_body_quat_i);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_I)) {
    INT32_RMAT_OF_EULERS(state.ned_to_body_rmat_i, state.ned_to_body_eulers_i);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_F)) {
    QUAT_BFP_OF_REAL(state.ned_to_body_quat_i, state.ned_to_body_quat_f);
    SetBit(state.att_status, ATT_QUAT_I);
    INT32_RMAT_OF_QUAT(state.ned_to_body_rmat_i, state.ned_to_body_quat_i);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_F)) {
    EULERS_BFP_OF_REAL(state.ned_to_body_eulers_i, state.ned_to_body_eulers_f);
    SetBit(state.att_status, ATT_EULER_I);
    INT32_RMAT_OF_EULERS(state.ned_to_body_rmat_i, state.ned_to_body_eulers_i);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.att_status, ATT_RMAT_I);

  //return state.ned_to_body_rmat_i;
}

void stateCalcNedToBodyEulers_i(void) {
  if (bit_is_set(state.att_status, ATT_EULER_I))
    return;

  if (bit_is_set(state.att_status, ATT_EULER_F)) {
    EULERS_BFP_OF_REAL(state.ned_to_body_eulers_i, state.ned_to_body_eulers_f);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_I)) {
    INT32_EULERS_OF_RMAT(state.ned_to_body_eulers_i, state.ned_to_body_rmat_i);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_I)) {
    INT32_EULERS_OF_QUAT(state.ned_to_body_eulers_i, state.ned_to_body_quat_i);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_F)) {
    RMAT_BFP_OF_REAL(state.ned_to_body_rmat_i, state.ned_to_body_rmat_f);
    SetBit(state.att_status, ATT_RMAT_I);
    INT32_EULERS_OF_RMAT(state.ned_to_body_eulers_i, state.ned_to_body_rmat_i);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_F)) {
    QUAT_BFP_OF_REAL(state.ned_to_body_quat_i, state.ned_to_body_quat_f);
    SetBit(state.att_status, ATT_QUAT_I);
    INT32_EULERS_OF_QUAT(state.ned_to_body_eulers_i, state.ned_to_body_quat_i);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.att_status, ATT_EULER_I);

  //return state.ned_to_body_eulers_i;
}

void stateCalcNedToBodyQuat_f(void) {
  if (bit_is_set(state.att_status, ATT_QUAT_F))
    return;

  if (bit_is_set(state.att_status, ATT_QUAT_I)) {
    QUAT_FLOAT_OF_BFP(state.ned_to_body_quat_f, state.ned_to_body_quat_i);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_F)) {
    FLOAT_QUAT_OF_RMAT(state.ned_to_body_quat_f, state.ned_to_body_rmat_f);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_F)) {
    FLOAT_QUAT_OF_EULERS(state.ned_to_body_quat_f, state.ned_to_body_eulers_f);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_I)) {
    RMAT_FLOAT_OF_BFP(state.ned_to_body_rmat_f, state.ned_to_body_rmat_i);
    SetBit(state.att_status, ATT_RMAT_F);
    FLOAT_QUAT_OF_RMAT(state.ned_to_body_quat_f, state.ned_to_body_rmat_f);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_I)) {
    EULERS_FLOAT_OF_BFP(state.ned_to_body_eulers_f, state.ned_to_body_eulers_i);
    SetBit(state.att_status, ATT_EULER_F);
    FLOAT_QUAT_OF_EULERS(state.ned_to_body_quat_f, state.ned_to_body_eulers_f);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.att_status, ATT_QUAT_F);

  //return state.ned_to_body_quat_f;
}

void stateCalcNedToBodyRMat_f(void) {
  if (bit_is_set(state.att_status, ATT_RMAT_F))
    return;

  if (bit_is_set(state.att_status, ATT_RMAT_I)) {
    RMAT_FLOAT_OF_BFP(state.ned_to_body_rmat_f, state.ned_to_body_rmat_i);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_F)) {
    FLOAT_RMAT_OF_QUAT(state.ned_to_body_rmat_i, state.ned_to_body_quat_i);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_F)) {
    FLOAT_RMAT_OF_EULERS(state.ned_to_body_rmat_i, state.ned_to_body_eulers_i);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_I)) {
    QUAT_FLOAT_OF_BFP(state.ned_to_body_quat_f, state.ned_to_body_quat_i);
    SetBit(state.att_status, ATT_QUAT_F);
    FLOAT_RMAT_OF_QUAT(state.ned_to_body_rmat_f, state.ned_to_body_quat_f);
  }
  else if (bit_is_set(state.att_status, ATT_EULER_I)) {
    EULERS_FLOAT_OF_BFP(state.ned_to_body_eulers_f, state.ned_to_body_eulers_i);
    SetBit(state.att_status, ATT_EULER_F);
    FLOAT_RMAT_OF_EULERS(state.ned_to_body_rmat_f, state.ned_to_body_eulers_f);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.att_status, ATT_RMAT_F);

  //return state.ned_to_body_rmat_f;
}

void stateCalcNedToBodyEulers_f(void) {
  if (bit_is_set(state.att_status, ATT_EULER_F))
    return;

  if (bit_is_set(state.att_status, ATT_EULER_I)) {
    EULERS_FLOAT_OF_BFP(state.ned_to_body_eulers_f, state.ned_to_body_eulers_i);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_F)) {
    FLOAT_EULERS_OF_RMAT(state.ned_to_body_eulers_f, state.ned_to_body_rmat_f);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_F)) {
    FLOAT_EULERS_OF_QUAT(state.ned_to_body_eulers_f, state.ned_to_body_quat_f);
  }
  else if (bit_is_set(state.att_status, ATT_RMAT_I)) {
    RMAT_FLOAT_OF_BFP(state.ned_to_body_rmat_f, state.ned_to_body_rmat_i);
    SetBit(state.att_status, ATT_RMAT_F);
    FLOAT_EULERS_OF_RMAT(state.ned_to_body_eulers_i, state.ned_to_body_rmat_i);
  }
  else if (bit_is_set(state.att_status, ATT_QUAT_I)) {
    QUAT_FLOAT_OF_BFP(state.ned_to_body_quat_f, state.ned_to_body_quat_i);
    SetBit(state.att_status, ATT_QUAT_F);
    FLOAT_EULERS_OF_QUAT(state.ned_to_body_eulers_f, state.ned_to_body_quat_f);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.att_status, ATT_EULER_F);

  //return state.ned_to_body_eulers_f;
}
/** @}*/


/******************************************************************************
 *                                                                            *
 * Transformation functions for the ANGULAR RATE representations              *
 *                                                                            *
 *****************************************************************************/
/** @addtogroup RateGroup
 *  @{ */

void stateCalcBodyRates_i(void) {
  if (bit_is_set(state.rate_status, RATE_I))
    return;

  if (bit_is_set(state.rate_status, RATE_F)) {
    RATES_BFP_OF_REAL(state.body_rates_i, state.body_rates_f);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.rate_status, RATE_I);

  //return state.body_rates_i;
}

void stateCalcBodyRates_f(void) {
  if (bit_is_set(state.rate_status, RATE_F))
    return;

  if (bit_is_set(state.rate_status, RATE_I)) {
    RATES_FLOAT_OF_BFP(state.body_rates_f, state.body_rates_i);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.rate_status, RATE_F);

  //return state.body_rates_f;
}

/** @}*/


/******************************************************************************
 *                                                                            *
 * Transformation functions for the WIND- AND AIRSPEED representations        *
 *                                                                            *
 *****************************************************************************/
/** @addtogroup WindAirGroup
 *  @{ */

void stateCalcHorizontalWindspeed_i(void) {
  if (bit_is_set(state.wind_air_status, WINDSPEED_I))
    return;

  if (bit_is_set(state.wind_air_status, WINDSPEED_F)) {
    state.h_windspeed_i.x = SPEED_BFP_OF_REAL(state.h_windspeed_f.x);
    state.h_windspeed_i.y = SPEED_BFP_OF_REAL(state.h_windspeed_f.y);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.rate_status, WINDSPEED_I);

  //return state.h_windspeed_i;
}

void stateCalcAirspeed_i(void) {
  if (bit_is_set(state.wind_air_status, AIRSPEED_I))
    return;

  if (bit_is_set(state.wind_air_status, AIRSPEED_F)) {
    state.airspeed_i = SPEED_BFP_OF_REAL(state.airspeed_f);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.wind_air_status, AIRSPEED_I);

  //return state.airspeed_i;
}

void stateCalcHorizontalWindspeed_f(void) {
  if (bit_is_set(state.wind_air_status, WINDSPEED_F))
    return;

  if (bit_is_set(state.wind_air_status, WINDSPEED_I)) {
    state.h_windspeed_f.x = SPEED_FLOAT_OF_BFP(state.h_windspeed_i.x);
    state.h_windspeed_f.x = SPEED_FLOAT_OF_BFP(state.h_windspeed_i.y);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.rate_status, WINDSPEED_F);

  //return state.h_windspeed_f;
}

void stateCalcAirspeed_f(void) {
  if (bit_is_set(state.wind_air_status, AIRSPEED_F))
    return;

  if (bit_is_set(state.wind_air_status, AIRSPEED_I)) {
    state.airspeed_f = SPEED_FLOAT_OF_BFP(state.airspeed_i);
  }
  /* set bit to indicate this representation is computed */
  SetBit(state.wind_air_status, AIRSPEED_F);

  //return state.airspeed_f;
}
/** @}*/
