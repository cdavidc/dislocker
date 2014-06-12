/* -*- coding: utf-8 -*- */
/* -*- mode: c -*- */
/*
 * Dislocker -- enables to read/write on BitLocker encrypted partitions under
 * Linux
 * Copyright (C) 2012-2013  Romain Coltel, Hervé Schauer Consultants
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include "print_metadata.h"


/**
 * Print a volume header structure into a human-readable format
 * 
 * @param volume_header The volume header to print
 */
void print_volume_header(LEVELS level, volume_header_t *volume_header)
{
	char rec_id[37];
	
	format_guid(volume_header->guid, rec_id);
	
	
	xprintf(level, "=====[ Volume header informations ]=====\n");
	xprintf(level, "  Signature: '%.8s'\n", volume_header->signature);
	xprintf(level, "  Sector size: 0x%1$04x (%1$hu) bytes\n", volume_header->sector_size);
	xprintf(level, "  Sector per cluster: 0x%1$02x (%1$hhu) bytes\n", volume_header->sectors_per_cluster);
	xprintf(level, "  Reserved clusters: 0x%1$04x (%1$hu) bytes\n", volume_header->reserved_clusters);
	xprintf(level, "  Fat count: 0x%1$02x (%1$hhu) bytes\n", volume_header->fat_count);
	xprintf(level, "  Root entries: 0x%1$04x (%1$hu) bytes\n", volume_header->root_entries);
	xprintf(level, "  Number of sectors (16 bits): 0x%1$04x (%1$hu) bytes\n", volume_header->nb_sectors_16b);
	xprintf(level, "  Media descriptor: 0x%1$02x (%1$hhu) bytes\n", volume_header->media_descriptor);
	xprintf(level, "  Sectors per fat: 0x%1$04x (%1$hu) bytes\n", volume_header->sectors_per_fat);
	xprintf(level, "  Hidden sectors: 0x%1$08x (%1$u) bytes\n", volume_header->hidden_sectors);
	xprintf(level, "  Number of sectors (32 bits): 0x%1$08x (%1$u) bytes\n", volume_header->nb_sectors_32b);
	xprintf(level, "  Number of sectors (64 bits): 0x%1$016x (%1$llu) bytes\n", volume_header->nb_sectors_64b);
	xprintf(level, "  MFT start cluster: 0x%1$016x (%1$lu) bytes\n", volume_header->mft_start_cluster);
	xprintf(level, "  Metadata Lcn: 0x%1$016x (%1$lu) bytes\n", volume_header->metadata_lcn);
	
	xprintf(level, "  Volume GUID: '%.37s'\n", rec_id);
	
	xprintf(level, "  First metadata header offset:  0x%016" F_U64_T "\n", volume_header->offset_bl_header[0]);
	xprintf(level, "  Second metadata header offset: 0x%016" F_U64_T "\n", volume_header->offset_bl_header[1]);
	xprintf(level, "  Third metadata header offset:  0x%016" F_U64_T "\n", volume_header->offset_bl_header[2]);
	
	xprintf(level, "  Boot Partition Identifier: '0x%04hx'\n", volume_header->boot_partition_identifier);
	xprintf(level, "========================================\n");
}


/**
 * Print a BitLocker header structure into a human-readable format
 * 
 * @param bl_header The BitLocker header to print
 */
void print_bl_metadata(LEVELS level, bitlocker_header_t *bl_header)
{
	xprintf(level, "=====================[ BitLocker metadata informations ]=====================\n");
	xprintf(level, "  Signature: '%.8s'\n", bl_header->signature);
	xprintf(level, "  Total Size: 0x%1$04x (%1$d) bytes (including signature and data)\n", bl_header->size << 4);
	xprintf(level, "  Version: %hu\n", bl_header->version);
	if(bl_header->version == V_SEVEN)
	{
		hexdump(level, bl_header->unknown1, 4);
		xprintf(level, "  Encrypted volume size: %1$llu bytes (%1$#llx), ~%2$llu MB\n", bl_header->encrypted_volume_size, bl_header->encrypted_volume_size / (1024*1024));
		hexdump(level, bl_header->unknown2, 4);
		xprintf(level, "  Number of boot sectors backuped: %1$u sectors (%1$#x)\n", bl_header->nb_backup_sectors);
	}
	else
		hexdump(level, bl_header->unknown1, 20);
#ifdef __ARCH_X86_64
	xprintf(level, "  First metadata header offset:  %#lx\n", bl_header->offset_bl_header[0]);
	xprintf(level, "  Second metadata header offset: %#lx\n", bl_header->offset_bl_header[1]);
	xprintf(level, "  Third metadata header offset:  %#lx\n", bl_header->offset_bl_header[2]);
	xprintf(level, "  Boot sectors backup address:   %#lx\n", bl_header->boot_sectors_backup);
#else
	xprintf(level, "  First metadata header offset:  %#llx\n", bl_header->offset_bl_header[0]);
	xprintf(level, "  Second metadata header offset: %#llx\n", bl_header->offset_bl_header[1]);
	xprintf(level, "  Third metadata header offset:  %#llx\n", bl_header->offset_bl_header[2]);
	xprintf(level, "  Boot sectors backup address:   %#llx\n", bl_header->boot_sectors_backup);
#endif
	
	print_dataset(level, &bl_header->dataset);
	xprintf(level, "=============================================================================\n");
}


/**
 * Print a BitLocker dataset structure into human-readable format
 * 
 * @param dataset The dataset to print
 */
void print_dataset(LEVELS level, bitlocker_dataset_t* dataset)
{
	time_t ts;
	char* date = NULL;
	char* cipher = cipherstr(dataset->algorithm);
	char formated_guid[37];
	
	format_guid(dataset->guid, formated_guid);
	ntfs2utc(dataset->timestamp, &ts);
	date = strdup(asctime(gmtime(&ts)));
	chomp(date);
	
	xprintf(level, "  ----------------------------{ Dataset header }----------------------------\n");
	xprintf(level, "    Dataset size: 0x%1$08x (%1$d) bytes (including data)\n", dataset->size);
	xprintf(level, "    Unknown data: 0x%08x (always 0x00000001)\n", dataset->unknown1);
	xprintf(level, "    Dataset header size: 0x%08x (always 0x00000030)\n", dataset->header_size);
	xprintf(level, "    Dataset copy size: 0x%1$08x (%1$d) bytes\n", dataset->copy_size);
	xprintf(level, "    Dataset GUID: '%.39s'\n", formated_guid);
	xprintf(level, "    Next counter: %u\n", dataset->next_counter);
	xprintf(level, "    Encryption Type: %s (%#hx)\n", cipher, dataset->algorithm);
	xprintf(level, "    Epoch Timestamp: %u sec, that to say %s\n", (unsigned int)ts, date);
	xprintf(level, "  --------------------------------------------------------------------------\n");
	
	xfree(cipher);
	free(date);
}


/**
 * Print data of a given metadata
 * 
 * @param metadata The metadata from where data will be printed
 */
void print_data(LEVELS level, void* metadata)
{
	// Check parameters
	if(!metadata)
		return;
	
	bitlocker_dataset_t* dataset = metadata + 0x40;
	void* data = NULL;
	void* end_dataset = 0;
	int loop = 0;
	
	/* Check this dataset validity */
	if(
		dataset->copy_size < dataset->header_size
		|| dataset->size   > dataset->copy_size
		|| dataset->copy_size - dataset->header_size < 8
	)
	{
		xprintf(L_ERROR, "Error, no dataset found.\n");
		return;
	}
	
	data = (char*)dataset + dataset->header_size;
	end_dataset = (char*)dataset + dataset->size;
	
	while(1)
	{
		/* Begin with reading the header */
		datum_header_safe_t header;
		
		if(data >= end_dataset)
			break;
		
		if(!get_header_safe(data, &header))
			break;
		
		if(data + header.datum_size > end_dataset)
			break;
		
		xprintf(level, "\n");
		xprintf(level, "======[ Datum n°%d informations ]======\n", ++loop);
		print_one_datum(level, data);
		xprintf(level, "=========================================\n");
		
		data += header.datum_size;
	}
}
