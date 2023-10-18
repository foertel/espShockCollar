import { BaseEntity, Column, Entity, PrimaryGeneratedColumn } from "typeorm";

@Entity()
export class Collar extends BaseEntity{
	@PrimaryGeneratedColumn()
    id: number

	@Column("varchar")
	identifier: string

	@Column("int")
	channel: number;

	@Column("int")
	receiverId: number;

	@Column("int")
	maximumIntensity: number;

	@Column("tinyint", {default: 0})
	isLinked: boolean;
}