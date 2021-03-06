/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi_gnvs.h>
#include <bootstate.h>
#include <cbmem.h>
#include <soc/acpi.h>
#include <soc/nvs.h>
#include <soc/southbridge.h>
#include <types.h>

static int get_index_bit(uint32_t value, uint16_t limit)
{
	uint16_t i;
	uint32_t t;

	if (limit >= TOTAL_BITS(uint32_t))
		return -1;

	/* get a mask of valid bits. Ex limit = 3, set bits 0-2 */
	t = (1 << limit) - 1;
	if ((value & t) == 0)
		return -1;
	t = 1;
	for (i = 0; i < limit; i++) {
		if (value & t)
			break;
		t <<= 1;
	}
	return i;
}

static void pm_fill_gnvs(const struct acpi_pm_gpe_state *state)
{
	int index;
	struct global_nvs *gnvs = acpi_get_gnvs();
	if (gnvs == NULL)
		return;

	index = get_index_bit(state->pm1_sts & state->pm1_en, PM1_LIMIT);
	if (index < 0)
		gnvs->pm1i = ~0ULL;
	else
		gnvs->pm1i = index;

	index = get_index_bit(state->gpe0_sts & state->gpe0_en, GPE0_LIMIT);
	if (index < 0)
		gnvs->gpei = ~0ULL;
	else
		gnvs->gpei = index;
}

static void set_nvs_sws(void *unused)
{
	struct chipset_state *state;

	state = cbmem_find(CBMEM_ID_POWER_STATE);
	if (state == NULL)
		return;

	pm_fill_gnvs(&state->gpe_state);
}

BOOT_STATE_INIT_ENTRY(BS_OS_RESUME, BS_ON_ENTRY, set_nvs_sws, NULL);
