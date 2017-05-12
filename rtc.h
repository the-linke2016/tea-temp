/*
 * rtc.h
 *
 *  Created on: 26 Nov 2014
 *      Author: mrowsell
 */

#ifndef RTC_H_
#define RTC_H_

void setupRTC() {
	RTCPS1CTL = RT1SSEL__ACLK + RT1PSDIV_6; // select ACLK for prescaler, divide by 128
	RTCPS1CTL &= ~RT1PSHOLD; 				// activate prescaler
	RTCCTL00 = RTCTEVIE; 					//enable time interrupt
	RTCCTL01 = RTCSSEL__RT1PS + RTCTEV_0;	// select prescaler as RTC clock
	RTCCTL01 &= ~RTCHOLD;					// activate RTC
}


#endif /* RTC_H_ */
