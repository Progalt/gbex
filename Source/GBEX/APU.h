
#ifndef GBEX_APU_H
#define GBEX_APU_H

#include "BitValue.h"

namespace gbex
{                  
	class APU
	{
	public:

	private:

		BitAddress m_Master;		// Master Volume and VIN Panning  - 0xFF24
		BitAddress m_SoundPanning;	// 0xFF25
		BitAddress m_Control;		// 0xFF26
	};
}

#endif // GBEX_APU_H