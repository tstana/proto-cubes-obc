/*
 * msp_i2c_common_due.h
 * Author: John Wikman
 *
 * Common definitions used by both the slave and master I2C implementation on
 * Arduino Due.
 *
 * Certain TWI functionalities does not have associated handler functions (like
 * when handling PDC). Consult the component_twi.h file and processor
 * specification for how to handle these cases.
 */

#ifndef MSP_I2C_COMMON_DUE_H
#define MSP_I2C_COMMON_DUE_H

#include <include/twi.h> // Atmel CMSIS Driver
#include "variant.h" // WIRE pins and definitions

#if WIRE_INTERFACES_COUNT < 1
	#error No WIRE interface found.
#endif

/* Pointer to TWI0 */
static Twi *twi = WIRE_INTERFACE;

static void msp_i2c_enable(void)
{
	NVIC_DisableIRQ(WIRE_ISR_ID);
	NVIC_ClearPendingIRQ(WIRE_ISR_ID);

	pmc_enable_periph_clk(WIRE_INTERFACE_ID);
	PIO_Configure(g_APinDescription[PIN_WIRE_SDA].pPort,
	              g_APinDescription[PIN_WIRE_SDA].ulPinType,
	              g_APinDescription[PIN_WIRE_SDA].ulPin,
	              g_APinDescription[PIN_WIRE_SDA].ulPinConfiguration);
	PIO_Configure(g_APinDescription[PIN_WIRE_SCL].pPort,
	              g_APinDescription[PIN_WIRE_SCL].ulPinType,
	              g_APinDescription[PIN_WIRE_SCL].ulPin,
	              g_APinDescription[PIN_WIRE_SCL].ulPinConfiguration);

	NVIC_SetPriority(WIRE_ISR_ID, 0);
	NVIC_EnableIRQ(WIRE_ISR_ID);

	/* Disable the PDC (Peripheral DMA Controller) */
	twi->TWI_PTCR = TWI_PTCR_RXTDIS | TWI_PTCR_TXTDIS;
}

static void msp_i2c_disable(void)
{
	/* Un-disable the PDC (Peripheral DMA Controller) */
	twi->TWI_PTCR &= ~(TWI_PTCR_RXTDIS | TWI_PTCR_TXTDIS);

	NVIC_DisableIRQ(WIRE_ISR_ID);
	NVIC_ClearPendingIRQ(WIRE_ISR_ID);

	pmc_disable_periph_clk(WIRE_INTERFACE_ID);
}

/*****************************************************************************
 * TWI_STATUS defines. There are already defines for TWI_SR_TXRDY and a few  *
 * others, but this ensures that there is a define for each possible status. *
 *****************************************************************************/
static inline int twi_status_isset(int sr, int mask)
{
	/* This statement should not be in a define, hence the function */
	return ((sr & mask) == mask);
}
#define TWI_STATUS_ISSET(sr, mask) twi_status_isset(sr, mask)

#ifndef TWI_STATUS_TXCOMP
#define TWI_STATUS_TXCOMP(sr) TWI_STATUS_ISSET(sr, TWI_SR_TXCOMP)
#endif

#ifndef TWI_STATUS_RXRDY
#define TWI_STATUS_RXRDY(sr) TWI_STATUS_ISSET(sr, TWI_SR_RXRDY)
#endif

#ifndef TWI_STATUS_TXRDY
#define TWI_STATUS_TXRDY(sr) TWI_STATUS_ISSET(sr, TWI_SR_TXRDY)
#endif

#ifndef TWI_STATUS_SVREAD
#define TWI_STATUS_SVREAD(sr) TWI_STATUS_ISSET(sr, TWI_SR_SVREAD)
#endif

#ifndef TWI_STATUS_SVACC
#define TWI_STATUS_SVACC(sr) TWI_STATUS_ISSET(sr, TWI_SR_SVACC)
#endif

#ifndef TWI_STATUS_GACC
#define TWI_STATUS_GACC(sr) TWI_STATUS_ISSET(sr, TWI_SR_GACC)
#endif

#ifndef TWI_STATUS_OVRE
#define TWI_STATUS_OVRE(sr) TWI_STATUS_ISSET(sr, TWI_SR_OVRE)
#endif

#ifndef TWI_STATUS_NACK
#define TWI_STATUS_NACK(sr) TWI_STATUS_ISSET(sr, TWI_SR_NACK)
#endif

#ifndef TWI_STATUS_ARBLST
#define TWI_STATUS_ARBLST(sr) TWI_STATUS_ISSET(sr, TWI_SR_ARBLST)
#endif

/* Unlike other defines, this is SCLWS instead of SCL_WS */
#ifndef TWI_STATUS_SCLWS
#define TWI_STATUS_SCLWS(sr) TWI_STATUS_ISSET(sr, TWI_SR_SCLWS)
#endif

#ifndef TWI_STATUS_EOSACC
#define TWI_STATUS_EOSACC(sr) TWI_STATUS_ISSET(sr, TWI_SR_EOSACC)
#endif

#ifndef TWI_STATUS_ENDRX
#define TWI_STATUS_ENDRX(sr) TWI_STATUS_ISSET(sr, TWI_SR_ENDRX)
#endif

#ifndef TWI_STATUS_ENDTX
#define TWI_STATUS_ENDTX(sr) TWI_STATUS_ISSET(sr, TWI_SR_ENDTX)
#endif

#ifndef TWI_STATUS_RXBUFF
#define TWI_STATUS_RXBUFF(sr) TWI_STATUS_ISSET(sr, TWI_SR_RXBUFF)
#endif

#ifndef TWI_STATUS_TXBUFE
#define TWI_STATUS_TXBUFE(sr) TWI_STATUS_ISSET(sr, TWI_SR_TXBUFE)
#endif

#endif /* MSP_I2C_COMMON_DUE_H */
