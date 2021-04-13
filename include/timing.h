/* timing facilities */

#ifndef TIMING_H_
#define TIMING_H_

typedef void (*PFN_timer_callback)(void);

static volatile PFN_timer_callback _timer1a_callback;

/* Timer1 is used with frequency divisor = 1024
 * It allows to count from 1 to ~65 seconds if F_CPU = 1000000 (1MHz) */
#define TIMER1_FREQDIV 1024
#define TIMER1_FREQDIV_MASK ((1 << CS12) | (1 << CS10))

uint32_t _timer1_ms_to_ocr(uint16_t timeout_ms);
void _timer1a_start_counting(uint16_t timeout_ms);
void _timer1b_start_counting(uint16_t timeout_ms);

void timer1_init()
{
	/* timer1 configuration */
	TCCR1B &= 0b11100000; // clear WGM and CS bits
	TCCR1B |= TIMER1_FREQDIV_MASK; // set frequency divisor
	TCCR1B |= 1 << WGM12; // CTC mode
}

/*  Starts timer1, callback will be invoked with timeout_ms period until
 * timer1_stop() is called.
 *  callback must be a valid pointer to function
 * Note. Global interrupts must be enabled to receive callbacks */
void timer1a_start_ms(uint16_t timeout_ms, PFN_timer_callback callback)
{
	_timer1a_callback = callback;
	_timer1a_start_counting(timeout_ms);
	TIMSK |= 1 << OCIE1A; //enable timer1a interrupts
}

/*  Note. timeout is set immediately, so new timeout value
 * should not be less than current TCCNT1 value (converted to ms).
 * Otherwise, the period before the first next callback may become bigger,
 * than it is supposed */
void timer1a_change_timeout_ms(uint16_t timeout_ms)
{
	OCR1A = _timer1_ms_to_ocr(timeout_ms); // set compare value
}

void timer1a_stop()
{
	TIMSK &= ~(1 << OCIE1A); // disable timer1a interrupts
}

void timer1a_wait_ms(uint16_t timeout_ms)
{
	timer1a_stop(); //disable timer1a interrupts
	_timer1a_start_counting(timeout_ms);
	while (TCNT1 < OCR1A)
		;
}

ISR(TIMER1_COMPA_vect) { _timer1a_callback(); }

uint32_t _timer1_ms_to_ocr(uint16_t timeout_ms)
{
	return ((uint32_t) (F_CPU / TIMER1_FREQDIV)) * timeout_ms / 1000; // 1000 - ms to sec
}

/* starts counting, not enabling interrupts */
void _timer1a_start_counting(uint16_t timeout_ms)
{
	OCR1A = _timer1_ms_to_ocr(timeout_ms); // set compare value
	TIFR &= ~(1 << OCF1A); // clear compare flag
	TCNT1 = 0; // set timer to 0
}

#endif // TIMING_H_