/*******************************************************************************
*
* Copyright 2015 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale License
* distributed with this Material.
* See the LICENSE file distributed for more details.
*
****************************************************************************//*!
*
* @file      SIN_LUT1024.h
*
* @author    B19186
* 
* @version   3.0.0.1
* 
* @date      June-5-2015
* 
* @brief     Header file for harmonic signals computing
*
*******************************************************************************/
#ifndef __SIN_LUT1024_H__
#define __SIN_LUT1024_H__

/*******************************************************************************
 * User data types and macro definitions                                       *
 ******************************************************************************/    
/*******************************************************************************
* Harmonic signals (sine/cosine) constants
*
*//*! @addtogroup harm_const
* @{
*******************************************************************************/
#define SIN_LUT_RES   32             ///< LUT resolution (16,24,32-bit)
#define SIN_SIZE      1024           ///< sin LUT size (number of points in the LUT)
#define SIN_LUT_MASK  (SIN_SIZE-1)   ///< mask for LUT 1024 positions: 0x0400-1 = 0x03ff
#define COS_OFFS      (SIN_SIZE/4)   ///< cosine is shifted by 90� to sine
#define TWO_PI        0              ///< goniometric functions output: 1=<0..+2PI), 0=(-PI..+PI>
/*! @} End of harm_const */

/*! Sine/cosine data types                                                                   */
#if SIN_LUT_RES == 16                    ///< Is LUT 16-bit?
    #define sin_lut_t short int       /*!< sine/cosine 16-bit data type */
#elif SIN_LUT_RES == 24                  ///< Is LUT 24-bit?
    #define sin_lut_t long            /*!< sine/cosine 24-bit data type */
#elif SIN_LUT_RES == 32                  ///< Is LUT 32-bit?
    #define sin_lut_t long            /*!< sine/cosine 32-bit data type */
#else                                ///< error
    #error("LUT definition error!"); 
#endif 
extern const sin_lut_t SinLUT[];      /*!< sine LUT (1024-points, 16/24/32-bit resolution) */

/*******************************************************************************
 * public function prototypes                                                  *
 ******************************************************************************/
/***************************************************************************//*!
*
* @brief  Sine computing
*
* @param  angle    Signed 32-bit integer value in thousandth degree in the range 
*                  <0..360000) or (-180000..+180000> selected by @ref TWO_PI
*         
* @return Signed 16/32-bit fractional value (-1...+1)
*
* @remarks Based on searching in sine LUT with subsequent linearization between
*          two points.
*
*******************************************************************************/
extern sin_lut_t Sine (long angle);

/***************************************************************************//*!
*
* @brief  Cosine computing
*
* @param  angle    Signed 32-bit integer value in thousandth degree in the range 
*                  <0..360000) or (-180000..+180000> selected by @ref TWO_PI
*         
* @return Signed 16/32-bit fractional value (-1...+1)
*
* @remarks Based on searching in sine LUT with subsequent linearization between
*          two points.
*
*******************************************************************************/
extern sin_lut_t Cosine (long angle);

#endif /* __SIN_LUT1024_H__ */